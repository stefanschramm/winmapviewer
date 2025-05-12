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

// Currently only one instance is supported!
ViewportRenderer* viewportRenderer;

// TODO: put these in the renderer?
bool dragging = FALSE;
int dragStartX = 0;
int dragStartY = 0;

LRESULT CALLBACK MapWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	LonLat* lonLat;

	switch (message) {
		case WM_CREATE:
			// TODO: Initialize service structure (ViewportRenderer etc.) in WM_CREATE if required (use factory method instead)
			// TODO: When multiple controls are active: common (static) TileCache, DownloadWorker, TileDownloader  and GdiWrapper instances, but individual ViewportRenderer
			// TODO: (statically) count controls to determine when common services can be destroyed
			viewportRenderer = new ViewportRenderer(5, hWnd);
			viewportRenderer->setCenterLonLat(13.377222, 52.526944);

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
			if (dragging) {
				viewportRenderer->setOffset(dragStartX - GET_X_LPARAM(lParam), dragStartY - GET_Y_LPARAM(lParam));
				InvalidateRect(hWnd, NULL, FALSE);
			}

			SendMessage(GetParent(hWnd), WM_MAP_LONLAT_UPDATE, 0, lParam);
			break;

		case WM_MAP_GET_LONLAT:
			lonLat = (LonLat*)wParam;
			viewportRenderer->getLonLat(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &(lonLat->lon), &(lonLat->lat));
			break;

		case WM_LBUTTONDOWN:
			dragStartX = GET_X_LPARAM(lParam);
			dragStartY = GET_Y_LPARAM(lParam);
			dragging = TRUE;
			SetCapture(hWnd);
			break;

		case WM_LBUTTONUP:
			viewportRenderer->moveToOffset();
			InvalidateRect(hWnd, NULL, FALSE);
			dragging = FALSE;
			ReleaseCapture();
			break;

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

		case WM_DESTROY:
			delete viewportRenderer;
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