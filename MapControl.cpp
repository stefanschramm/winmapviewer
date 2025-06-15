#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

#include "DownloadWorker.h"
#include "GdiPlusWrapper.h"
#include "MapControl.h"
#include "TileCache.h"
#include "TileDownloader.h"
#include "ViewportRenderer.h"

// Compatibility with VC++6
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#define IDM_COPY_LON_LAT 10001

std::map<HWND, ViewportRenderer*> renderers;

LonLat clickLonLat;

void putTextIntoClipboard(char* text);
void printMap(HWND hWnd, ViewportRenderer* const viewportRenderer);

LRESULT CALLBACK MapWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	LonLat* lonLat;
	ViewportRenderer* viewportRenderer = NULL;

	if (message != WM_CREATE && !renderers.count(hWnd)) {
		// should not happen
		return DefWindowProc(hWnd, message, wParam, lParam);
	} else {
		viewportRenderer = renderers[hWnd];
	}

	switch (message) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_COPY_LON_LAT: {
					char latLonTxt[256];
					sprintf(latLonTxt, TEXT("%.8f %.8f"), clickLonLat.lat, clickLonLat.lon);
					putTextIntoClipboard(latLonTxt);
					break;
				}

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_CREATE:
			viewportRenderer = new ViewportRenderer(5, hWnd);
			viewportRenderer->setCenterLonLat(13.377222, 52.526944);
			renderers[hWnd] = viewportRenderer;

			break;

		case WM_SIZE:
			viewportRenderer->setViewportSize(LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_ERASEBKGND:
			// reduce flickering
			return TRUE;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Improve rendering - only render part that needs updating
			viewportRenderer->render(hdc);
			EndPaint(hWnd, &ps);
			break;

		case WM_USER_TILE_READY:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: render only tile(s) that have finished downloading
			InvalidateRect(hWnd, NULL, FALSE);
			EndPaint(hWnd, &ps);
			break;

		case WM_MOUSEMOVE:
			if (viewportRenderer->mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
				InvalidateRect(hWnd, NULL, FALSE);
			}
			SendMessage(GetParent(hWnd), WM_MAP_LONLAT_UPDATE, 0, lParam);
			break;

		case WM_MAP_GET_LONLAT:
			lonLat = (LonLat*)wParam;
			viewportRenderer->getLonLat(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &(lonLat->lon), &(lonLat->lat));
			break;

		case WM_LBUTTONDOWN:
			viewportRenderer->startDragging(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			viewportRenderer->endDragging(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			InvalidateRect(hWnd, NULL, FALSE);
			ReleaseCapture();
			break;

		case WM_RBUTTONDOWN: {
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, IDM_COPY_LON_LAT, "Copy coordinates to clipboard (Format: lat lon)");
			POINT pt;
			GetCursorPos(&pt);
			viewportRenderer->getLonLat(pt.x, pt.y, &clickLonLat.lon, &clickLonLat.lat);
			TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
			break;
		}

		case WM_MOUSEWHEEL:
			// TODO: Pass position and adjust new center; coords may be shifted when widget is not at 0;0
			if ((short)HIWORD(wParam) > 0) {
				viewportRenderer->zoomIn();
			} else {
				viewportRenderer->zoomOut();
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MAP_ZOOM_IN:
			viewportRenderer->zoomIn();
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MAP_ZOOM_OUT:
			viewportRenderer->zoomOut();
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MAP_MOVE_X:
			viewportRenderer->setOffset(wParam, 0);
			viewportRenderer->moveToOffset();
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MAP_MOVE_Y:
			viewportRenderer->setOffset(0, wParam);
			viewportRenderer->moveToOffset();
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_MAP_PRINT:
			printMap(hWnd, viewportRenderer);
			break;

		case WM_DESTROY:
			delete viewportRenderer;
			renderers.erase(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

void RegisterMapControl(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)MapWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("MapControl");
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, TEXT("Error registering map control"), TEXT("winmapviewer"), MB_OK);
	}
}

HWND CreateMapWindow(int x, int y, int width, int height, HWND hWnd, HINSTANCE hInstance) {
	return CreateWindowEx(
		0,
		TEXT("MapControl"),
		NULL,
		WS_CHILD | WS_VISIBLE,
		x,
		y,
		width,
		height,
		hWnd,
		NULL,
		hInstance,
		NULL
	);
}

void putTextIntoClipboard(char* text) {
	if (!OpenClipboard(NULL)) {
		return;
	}
	EmptyClipboard();
	size_t size = strlen(text) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
	if (hMem) {
		char* pMem = (char*)GlobalLock(hMem);
		if (pMem) {
			memcpy(pMem, text, size);
			GlobalUnlock(hMem);
			SetClipboardData(CF_TEXT, hMem);
		}
	}
	CloseClipboard();
}

void printMap(HWND hWnd, ViewportRenderer* const viewportRenderer) {
    PRINTDLG pd;
    memset(&pd, 0, sizeof(pd));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.hwndOwner = hWnd;
    pd.Flags = PD_RETURNDC | PD_PRINTSETUP;
    pd.nCopies = 1;

    if (!PrintDlg(&pd)) {
		return;
	}

	HDC hdcPrint = pd.hDC;
	static DOCINFO di = {sizeof(DOCINFO), TEXT("WinMapViewer")};
	if (hdcPrint == NULL) {
		return;
	}

	// TODO: Set abort procedure
	// TODO: Display modal dialog with cancel button

	if (StartDoc(hdcPrint, &di) <= 0) {
		MessageBox(NULL, TEXT("StartDoc problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}
	if (StartPage(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("StartPage problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}

	// TODO: Printing commands
	viewportRenderer->render(hdcPrint);

	if (EndPage(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("EndPage problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}
	if (EndDoc(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("EndDoc problem"), TEXT("winmapviewer"), MB_OK);
	}
	DeleteDC(hdcPrint);
}