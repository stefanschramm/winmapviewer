#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "resource.h"

#include "DownloadWorker.h"
#include "GdiPlusWrapper.h"
#include "TileCache.h"
#include "TileDownloader.h"
#include "ViewportRenderer.h"

#define MAX_LOADSTRING 100

// Compatibility with VC++6
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

const int ARROW_KEYS_MOVE_DISTANCE = 40;

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HWND hwndStatus;
bool dragging = FALSE;
int dragStartX = 0;
int dragStartY = 0;

ViewportRenderer* viewportRenderer;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINMAPVIEWER, szWindowClass, MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	InitCommonControls();

	// init instance
	hInst = hInstance;

	HWND hWnd = CreateWindowEx(
	  WS_EX_CLIENTEDGE,
	  szWindowClass,
	  szTitle,
	  WS_OVERLAPPEDWINDOW,
	  CW_USEDEFAULT,
	  0,
	  CW_USEDEFAULT,
	  0,
	  NULL,
	  NULL,
	  hInstance,
	  NULL
	);

	if (!hWnd) {
		return FALSE;
	}

	hwndStatus = CreateWindowEx(
	  0,
	  STATUSCLASSNAME,
	  NULL,
	  WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
	  0,
	  0,
	  0,
	  0,
	  hWnd,
	  (HMENU)100,
	  hInst,
	  NULL
	);

	int partSizes[2] = {100, 200};

	SendMessage(hwndStatus, SB_SETPARTS, sizeof(partSizes), (LPARAM)partSizes);

	GdiPlusWrapper gdi;
	TileDownloader tileDownloader(gdi);
	DownloadWorker downloadWorker(tileDownloader, hWnd);
	TileCache tileCache(tileDownloader, downloadWorker);
	viewportRenderer = new ViewportRenderer(tileCache, 5);
	viewportRenderer->setCenterLonLat(13.377222, 52.526944);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WINMAPVIEWER);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete viewportRenderer;

	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_WINMAPVIEWER);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = (LPCSTR)IDC_WINMAPVIEWER;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	int wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	char statusText[128];
	double lon;
	double lat;

	switch (message) {
		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId) {
				case IDM_ABOUT:
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
					break;

				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;

				case IDM_ZOOMIN:
					viewportRenderer->zoomIn();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case IDM_ZOOMOUT:
					viewportRenderer->zoomOut();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case IDM_RIGHT:
					viewportRenderer->setOffset(ARROW_KEYS_MOVE_DISTANCE, 0);
					viewportRenderer->moveToOffset();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case IDM_LEFT:
					viewportRenderer->setOffset(-ARROW_KEYS_MOVE_DISTANCE, 0);
					viewportRenderer->moveToOffset();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case IDM_UP:
					viewportRenderer->setOffset(0, -ARROW_KEYS_MOVE_DISTANCE);
					viewportRenderer->moveToOffset();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				case IDM_DOWN:
					viewportRenderer->setOffset(0, ARROW_KEYS_MOVE_DISTANCE);
					viewportRenderer->moveToOffset();
					InvalidateRect(hWnd, NULL, FALSE);
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_USER_TILE_READY:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: render only tile(s) that have finished downloading
			InvalidateRect(hWnd, NULL, FALSE);
			EndPaint(hWnd, &ps);

			break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Improve rendering - only render part that needs updating
			// TODO: Don't render onto status bar area
			viewportRenderer->render(hdc);
			EndPaint(hWnd, &ps);
			break;

		case WM_ERASEBKGND:
			// reduce flickering
			return TRUE;

		case WM_SIZE:
			viewportRenderer->setViewportSize(LOWORD(lParam), HIWORD(lParam));
			SendMessage(hwndStatus, WM_SIZE, 0, 0);
			break;

		case WM_MOUSEMOVE:
			if (dragging) {
				viewportRenderer->setOffset(dragStartX - GET_X_LPARAM(lParam), dragStartY - GET_Y_LPARAM(lParam));
				InvalidateRect(hWnd, NULL, FALSE);
			}

			viewportRenderer->getLonLat(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &lon, &lat);
			sprintf(statusText, TEXT("lon: %.6f"), lon);
			SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)statusText);
			sprintf(statusText, TEXT("lat: %.6f"), lat);
			SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)statusText);

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
			// TODO: Pass position and adjust new center
			if ((short)HIWORD(wParam) > 0) {
				viewportRenderer->zoomIn();
			} else {
				viewportRenderer->zoomOut();
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
	return FALSE;
}
