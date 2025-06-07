// windows.h is required to be included *before* commctrl.h
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <windowsx.h>

#include "resource.h"

#include "MapControl.h"

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
HWND hwndMap;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MapWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINMAPVIEWER, szWindowClass, MAX_LOADSTRING);

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

	if (!RegisterClassEx(&wcex)) {
		MessageBox(NULL, TEXT("Error registering main window"), TEXT("winmapviewer"), MB_OK);
	}

	RegisterMapControl(hInstance);

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

	hwndStatus = CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		TEXT(""),
		hWnd,
		100
	);

	hwndMap = CreateMapWindow(0, 0, 200, 200, hWnd, hInstance);

	// CreateMapWindow(210, 0, 200, 200, hWnd, hInstance);

	int partSizes[2] = {100, 200};

	SendMessage(hwndStatus, SB_SETPARTS, sizeof(partSizes), (LPARAM)partSizes);

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

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	int wmEvent;
	char statusText[128];
	LonLat lonLat;

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
					SendMessage(hwndMap, WM_MAP_ZOOM_IN, 0, 0);
					break;

				case IDM_ZOOMOUT:
					SendMessage(hwndMap, WM_MAP_ZOOM_OUT, 0, 0);
					break;

				case IDM_RIGHT:
					SendMessage(hwndMap, WM_MAP_MOVE_X, ARROW_KEYS_MOVE_DISTANCE, 0);
					break;

				case IDM_LEFT:
					SendMessage(hwndMap, WM_MAP_MOVE_X, -ARROW_KEYS_MOVE_DISTANCE, 0);
					break;

				case IDM_UP:
					SendMessage(hwndMap, WM_MAP_MOVE_Y, -ARROW_KEYS_MOVE_DISTANCE, 0);
					break;

				case IDM_DOWN:
					SendMessage(hwndMap, WM_MAP_MOVE_Y, ARROW_KEYS_MOVE_DISTANCE, 0);
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_ERASEBKGND:
			// reduce flickering
			return TRUE;

		case WM_SIZE:
			SendMessage(hwndStatus, WM_SIZE, 0, 0);
			RECT rect;
			SendMessage(hwndStatus, SB_GETRECT, 0, (LPARAM)&rect);
			MoveWindow(hwndMap, 0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) - (rect.bottom - rect.top + 2), TRUE);
			break;

		case WM_MOUSEWHEEL:
			SendMessage(hwndMap, WM_MOUSEWHEEL, wParam, lParam);
			break;

		case WM_MAP_LONLAT_UPDATE:
			SendMessage(hwndMap, WM_MAP_GET_LONLAT, (WPARAM)&lonLat, (LPARAM)lParam);
			sprintf(statusText, TEXT("lon: %.6f"), lonLat.lon);
			SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)statusText);
			sprintf(statusText, TEXT("lat: %.6f"), lonLat.lat);
			SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)statusText);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

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
