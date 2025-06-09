#include <math.h>

#include "ViewportRenderer.h"

// used for optimized multiplication
const int TILE_SIZE_BITS = 8;

// == 256 px
const int TILE_SIZE = 1 << TILE_SIZE_BITS;
const int TILE_INNER_OFFSET_MAP = 0xff;

// M_PI and asinh are missing in math.h of VC++ 6
#if !defined M_PI
const double M_PI = 3.141592653589793;
#endif

double asinh(double x) {
	return log(x + sqrt(x * x + 1));
}

ViewportRenderer::ViewportRenderer(int zoomLevel, HWND hWnd)
	: m_viewportWidth(640),
	  m_viewportHeight(480),
	  m_offsetX(0),
	  m_offsetY(0),
	  m_zoomLevel(zoomLevel),
	  m_x(0),
	  m_y(0) {
	m_gdi = new GdiPlusWrapper();
	// TODO: use same instance for every ViewportRenderer
	m_tileDownloader = new TileDownloader(m_gdi);
	m_downloadWorker = new DownloadWorker(m_tileDownloader, hWnd);
	m_tileCache = new TileCache(m_tileDownloader, m_downloadWorker);
}

ViewportRenderer::~ViewportRenderer() {
	delete m_tileCache;
	delete m_downloadWorker;
	delete m_tileDownloader;
	delete m_gdi;
}

void ViewportRenderer::render(HDC hdcDestination) {
	// top left corner of complete map
	long originX = m_x + m_offsetX;
	long originY = m_y + m_offsetY;

	restrictCoordinates(&originX, &originY);

	// top left tile
	int originTileX = originX >> TILE_SIZE_BITS;
	int originTileY = originY >> TILE_SIZE_BITS;

	// offset within the top left tile
	long offsetX = originX & TILE_INNER_OFFSET_MAP;
	long offsetY = originY & TILE_INNER_OFFSET_MAP;

	HDC hMemDC = CreateCompatibleDC(hdcDestination);

	// render one additional row/column of tiles at each edge
	for (int x = 0; x < (m_viewportWidth >> TILE_SIZE_BITS) + 2; x++) {
		for (int y = 0; y < (m_viewportHeight >> TILE_SIZE_BITS) + 2; y++) {
			int tileX = (originTileX + x) % (1 << m_zoomLevel);
			int tileY = originTileY + y;
			if (tileY > (1 << m_zoomLevel) - 1) {
				// south out of bounds
				RECT rect = {
					-offsetX + (x << TILE_SIZE_BITS),
					-offsetY + (y << TILE_SIZE_BITS),
					-offsetX + (x << TILE_SIZE_BITS) + TILE_SIZE,
					-offsetY + (y << TILE_SIZE_BITS) + TILE_SIZE};
				HBRUSH hBrush = CreateSolidBrush(RGB(128, 128, 128));
				FillRect(hdcDestination, &rect, hBrush);
				DeleteObject(hBrush);

				continue;
			}

			TileKey tileKey(m_zoomLevel, tileX, tileY);

			HBITMAP hBitmap = m_tileCache->get(tileKey);
			SelectObject(hMemDC, hBitmap);
			BitBlt(
				hdcDestination,
				-offsetX + (x << TILE_SIZE_BITS),
				-offsetY + (y << TILE_SIZE_BITS),
				TILE_SIZE,
				TILE_SIZE,
				hMemDC,
				0,
				0,
				SRCCOPY
			);
		}
	}

	DeleteDC(hMemDC);
}

void ViewportRenderer::setOffset(int offsetX, int offsetY) {
	m_offsetX = offsetX;
	m_offsetY = offsetY;
}

void ViewportRenderer::moveToOffset() {
	m_x += m_offsetX;
	m_y += m_offsetY;
	restrictCoordinates(&m_x, &m_y);
	m_offsetX = 0;
	m_offsetY = 0;
}

void ViewportRenderer::setCenterLonLat(double lon, double lat) {
	long mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;
	m_x = mapSize * (lon + 180.0) / 360.0 - (m_viewportWidth >> 1);
	m_y = mapSize * (1.0 - asinh(tan(lat * M_PI / 180.0)) / M_PI) / 2.0 - (m_viewportHeight >> 1);
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::zoomIn() {
	if (m_zoomLevel >= 19) {
		return;
	}

	m_zoomLevel++;
	m_x = m_x << 1;
	m_y = m_y << 1;
	m_x = m_x + (m_viewportWidth >> 1);
	m_y = m_y + (m_viewportHeight >> 1);
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::zoomOut() {
	if (m_zoomLevel <= 0) {
		return;
	}

	m_zoomLevel--;
	m_x = m_x - (m_viewportWidth >> 1);
	m_y = m_y - (m_viewportHeight >> 1);
	m_x = m_x >> 1;
	m_y = m_y >> 1;
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::setViewportSize(int width, int height) {
	// re-center
	// m_x = m_x - ((width - m_viewportWidth) / 2.0);
	// m_y = m_y - ((height - m_viewportHeight) / 2.0);
	// restrictCoordinates(&m_x, &m_y);

	m_viewportWidth = width;
	m_viewportHeight = height;
}

void ViewportRenderer::getLonLat(int x, int y, double* lon, double* lat) const {
	double mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;
	*lon = (m_x + x + m_offsetX) / mapSize * 360.0 - 180.0;
	*lat = atan(sinh(M_PI * (1.0 - 2.0 * (m_y + y + m_offsetY) / mapSize))) * 180.0 / M_PI;
}

void ViewportRenderer::restrictCoordinates(long* x, long* y) const {
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

void ViewportRenderer::startDragging(int x, int y) {
	m_dragging = true;
	m_dragStartX = x;
	m_dragStartY = y;
}

bool ViewportRenderer::mouseMove(int x, int y) {
	if (m_dragging) {
		setOffset(m_dragStartX - x, m_dragStartY - y);
		return true;
	}

	return false;
}

void ViewportRenderer::endDragging(int x, int y) {
	setOffset(m_dragStartX - x, m_dragStartY - y);
	moveToOffset();
	m_dragging = false;
}
