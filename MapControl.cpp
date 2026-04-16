#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

#include "Common.h"
#include "MapControl.h"
#include "TileIterator.h"
#include "TileRange.h"

#define IDM_COPY_LON_LAT 10001

void putTextIntoClipboard(char* text);

// used for optimized multiplication
const int TILE_SIZE_BITS = 8;

// == 256 px
const int TILE_SIZE = 1 << TILE_SIZE_BITS;
const int TILE_INNER_OFFSET_MAP = 0xff;

// VC++ 6 compatibility
#if !defined M_PI
const double M_PI = 3.141592653589793;
#endif

// VC++ 6 compatibility: asinh is missing in math.h
double asinh(double x) {
	return log(x + sqrt(x * x + 1));
}

MapControl::MapControl(HINSTANCE hInstance, HWND hwndMain, TileCache& tileCache)
	: m_hInstance(hInstance),
	  m_hwndMain(hwndMain),
	  m_tileCache(tileCache),
	  m_offsetX(0),
	  m_offsetY(0),
	  m_zoomLevel(0),
	  m_x(0),
	  m_y(0),
	  m_dragging(false),
	  m_styleUrlTemplate("http://osm.kesto.de/tile/osm/{z}/{x}/{y}.png") {
	m_unmappedBrush = CreateSolidBrush(RGB(128, 128, 128));
}

MapControl::~MapControl() {
	DeleteObject(m_unmappedBrush);
}

LRESULT CALLBACK MapControl::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	try {
		switch (message) {
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
					case IDM_COPY_LON_LAT: {
						char latLonTxt[256];
						sprintf(latLonTxt, TEXT("%.8f %.8f"), m_clickLonLat.lat, m_clickLonLat.lon);
						putTextIntoClipboard(latLonTxt);
						break;
					}

					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;

			case WM_CREATE: {
				m_hwndMap = hWnd;

				RECT clientRect;
				GetClientRect(m_hwndMap, &clientRect);
				m_viewportWidth = clientRect.right;
				m_viewportHeight = clientRect.bottom;

				break;
			}

			case WM_SIZE:
				setViewportSize(LOWORD(lParam), HIWORD(lParam));
				break;

			case WM_ERASEBKGND:
				// reduce flickering
				return TRUE;

			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc;
				RECT updateRect;
				BOOL hasUpdateRect = GetUpdateRect(hWnd, &updateRect, false);
				hdc = BeginPaint(hWnd, &ps);
				render(hdc, hasUpdateRect ? &updateRect : NULL);
				EndPaint(hWnd, &ps);
				break;
			}

			case WM_USER_TILE_AVAILABLE: {
				const TileKey* updatedTile = reinterpret_cast<const TileKey*>(lParam);
				invalidateUpdateRects(*updatedTile);
				break;
			}

			case WM_MOUSEMOVE: {
				if (mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
					InvalidateRect(hWnd, NULL, FALSE);
				}
				LonLat myLonLat;
				getLonLat(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &myLonLat);
				SendMessage(m_hwndMain, WM_USER_MAP_LONLAT_UPDATE, 0, reinterpret_cast<LPARAM>(&myLonLat));
				break;
			}

			case WM_LBUTTONDOWN:
				startDragging(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				SetCapture(hWnd);
				break;

			case WM_LBUTTONUP:
				endDragging(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				requestRedraw();
				ReleaseCapture();
				break;

			case WM_RBUTTONDOWN: {
				getLonLat(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &m_clickLonLat);

				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDM_COPY_LON_LAT, "Copy coordinates to clipboard (Format: lat lon)");
				POINT absolutePosition;
				GetCursorPos(&absolutePosition);
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, absolutePosition.x, absolutePosition.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
				break;
			}

			case WM_NCDESTROY:
				delete this;
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} catch (char const* e) {
		panicMessage("map control window procedure", e);
		exit(EXIT_FAILURE);
	}

	return FALSE;
}

HWND MapControl::create(int x, int y, int width, int height) {
	static bool registered = false;
	if (!registered) {
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &WndProcStaticHelper<MapControl>::wndProcStatic;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = TEXT("MapControl");
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex)) {
			throw "Error registering map control";
		}

		registered = true;
	}

	m_hwndMap = CreateWindowEx(
		0,
		TEXT("MapControl"),
		NULL,
		WS_CHILD | WS_VISIBLE,
		x,
		y,
		width,
		height,
		m_hwndMain,
		NULL,
		m_hInstance,
		this
	);

	return m_hwndMap;
}

void MapControl::requestRedraw() {
	InvalidateRect(m_hwndMap, NULL, FALSE);
}

void MapControl::render(HDC hdcDestination, RECT* updateRect) {
	long originX = m_x + m_offsetX;
	long originY = m_y + m_offsetY;
	restrictCoordinates(&originX, &originY);

	TileIterator tileIterator(m_zoomLevel, originX, originY, m_viewportWidth, m_viewportHeight);

	m_tileCache.unqueueInvisible(
		tileIterator.getTileRange(m_styleUrlTemplate),
		m_hwndMap
	);

	HDC hMemDC = CreateCompatibleDC(hdcDestination);

	int tileX;
	int tileY;
	RECT tileRect;
	while (tileIterator.next(&tileX, &tileY, &tileRect)) {
		RECT intersectRect;
		if (updateRect != NULL && !IntersectRect(&intersectRect, &tileRect, updateRect)) {
			continue;
		}

		TileKey tileKey(m_styleUrlTemplate, m_zoomLevel, tileX, tileY);

		HBITMAP hBitmap = m_tileCache.get(tileKey, m_hwndMap);
		SelectObject(hMemDC, hBitmap);
		BitBlt(
			hdcDestination,
			tileRect.left,
			tileRect.top,
			TILE_SIZE,
			TILE_SIZE,
			hMemDC,
			0,
			0,
			SRCCOPY
		);
	}

	// On low zoom levels (or large viewports) there is an unmapped area at the bottom (south)
	int outOfRangeYOffset = tileIterator.getOutOfRangeYOffset();
	if (outOfRangeYOffset > 0) {
		RECT outOfBoundsRect;
		outOfBoundsRect.left = 0;
		outOfBoundsRect.right = m_viewportWidth;
		outOfBoundsRect.top = outOfRangeYOffset;
		outOfBoundsRect.bottom = m_viewportHeight;
		FillRect(hdcDestination, &outOfBoundsRect, m_unmappedBrush);
	}

	DeleteDC(hMemDC);
}

void MapControl::setOffset(int offsetX, int offsetY) {
	m_offsetX = offsetX;
	m_offsetY = offsetY;
}

void MapControl::moveToOffset() {
	m_x += m_offsetX;
	m_y += m_offsetY;
	restrictCoordinates(&m_x, &m_y);
	m_offsetX = 0;
	m_offsetY = 0;
}

void MapControl::setCenterLonLat(const LonLat* lonLat) {
	long mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;
	m_x = mapSize * (lonLat->lon + 180.0) / 360.0 - (m_viewportWidth >> 1);
	m_y = mapSize * (1.0 - asinh(tan(lonLat->lat * M_PI / 180.0)) / M_PI) / 2.0 - (m_viewportHeight >> 1);
	restrictCoordinates(&m_x, &m_y);
}

void MapControl::setZoomLevel(int zoomLevel) {
	setZoomLevelKeepingFixPoint(zoomLevel, m_viewportWidth >> 1, m_viewportHeight >> 1);
}

void MapControl::setZoomLevelKeepingFixPoint(int zoomLevel, int x, int y) {
	if (zoomLevel == m_zoomLevel || zoomLevel < 0) {
		return;
	}

	int absoluteX = m_x + x;
	int absoluteY = m_y + y;

	int deltaZoomLevel = zoomLevel - m_zoomLevel;
	if (deltaZoomLevel > 0) {
		absoluteX <<= deltaZoomLevel;
		absoluteY <<= deltaZoomLevel;
	} else if (deltaZoomLevel < 0) {
		absoluteX >>= -deltaZoomLevel;
		absoluteY >>= -deltaZoomLevel;
	}

	m_x = absoluteX - x;
	m_y = absoluteY - y;
	m_zoomLevel = zoomLevel;

	restrictCoordinates(&m_x, &m_y);
}

void MapControl::setViewportSize(int width, int height) {
	m_viewportWidth = width;
	m_viewportHeight = height;
}

void MapControl::getLonLat(int x, int y, LonLat* lonLat) const {
	int mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;
	lonLat->lon = ((m_x + x + m_offsetX) % mapSize) / (double)mapSize * 360.0 - 180.0;
	lonLat->lat = atan(sinh(M_PI * (1.0 - 2.0 * (m_y + y + m_offsetY) / mapSize))) * 180.0 / M_PI;
}

void MapControl::getSettings(Settings* settings) const {
	settings->centerX = m_x + m_viewportWidth / 2;
	settings->centerY = m_y + m_viewportHeight / 2;
	settings->zoomLevel = m_zoomLevel;
}

void MapControl::setSettings(Settings* settings) {
	m_x = settings->centerX - m_viewportWidth / 2;
	m_y = settings->centerY - m_viewportHeight / 2;
	m_zoomLevel = settings->zoomLevel;
	restrictCoordinates(&m_x, &m_y);
}

void MapControl::restrictCoordinates(long* x, long* y) const {
	long mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;

	// normalize x position
	if (*x < 0) {
		*x = (1 + mapSize) + *x;
	}
	*x = *x % mapSize;

	// restrict y position
	// Note: Depending on the viewportHeight and zoomLevel there still will be undrawn areas at the bottom.
	if (*y + m_viewportHeight > mapSize) {
		*y = mapSize - m_viewportHeight;
	}
	if (*y < 0) {
		*y = 0;
	}
}

// Invalidate areas in which the tile is visible. On low zoom levels a tile can be visible multiple times due to the wrapping of the map.
void MapControl::invalidateUpdateRects(const TileKey& tileKey) const {
	long originX = m_x + m_offsetX;
	long originY = m_y + m_offsetY;
	restrictCoordinates(&originX, &originY);

	TileIterator tileIterator(m_zoomLevel, originX, originY, m_viewportWidth, m_viewportHeight);

	int tileX;
	int tileY;
	RECT tileRect;
	while (tileIterator.next(&tileX, &tileY, &tileRect)) {
		if (tileKey.x == tileX && tileKey.y == tileY) {
			InvalidateRect(m_hwndMap, &tileRect, FALSE);
		}
	}
}

void MapControl::startDragging(int x, int y) {
	m_dragging = true;
	m_dragStartX = x;
	m_dragStartY = y;
}

bool MapControl::mouseMove(int x, int y) {
	if (m_dragging) {
		setOffset(m_dragStartX - x, m_dragStartY - y);
		return true;
	}

	return false;
}

void MapControl::endDragging(int x, int y) {
	setOffset(m_dragStartX - x, m_dragStartY - y);
	moveToOffset();
	m_dragging = false;
}

void MapControl::setStyle(const std::string& styleUrlTemplate) {
	m_styleUrlTemplate = styleUrlTemplate;
}

void putTextIntoClipboard(char* text) {
	if (!OpenClipboard(NULL)) {
		return;
	}
	EmptyClipboard();
	size_t size = strlen(text) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
	if (hMem) {
		char* pMem = reinterpret_cast<char*>(GlobalLock(hMem));
		if (pMem) {
			memcpy(pMem, text, size);
			GlobalUnlock(hMem);
			SetClipboardData(CF_TEXT, hMem);
		}
	}
	CloseClipboard();
}
