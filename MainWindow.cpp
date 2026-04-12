// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <iostream>
#include <windowsx.h>

#include "Common.h"
#include "MainWindow.h"
#include "SearchDialog.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

// Compatibility with VC++6
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

const int ARROW_KEYS_MOVE_DISTANCE = 40;

const int STATUS_BAR_CONTROL_IDENTIFIER = 1001;

const int STATUS_BAR_PART_LON = 0;
const int STATUS_BAR_PART_LAT = 1;
const int STATUS_BAR_PART_ZOOM = 2;
const int STATUS_BAR_PART_ATTRIBUTION = 3;

const int CUSTOM_STYLE_MAX_ZOOM_LEVEL = 30;

int MainWindow::mainWindowCount = 0;

MainWindow::MainWindow(
	HINSTANCE hInstance,
	const StyleDatabase& styleDatabase,
	Settings settings,
	TileCache& tileCache
) : m_hInstance(hInstance),
	m_styleDatabase(styleDatabase),
	m_settings(settings),
	m_tileCache(tileCache) {
}

bool MainWindow::create(int nCmdShow) {
	static bool mainWindowIsRegistered = false;

	if (!mainWindowIsRegistered) {
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &WndProcStaticHelper<MainWindow>::wndProcStatic;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = LoadIcon(m_hInstance, reinterpret_cast<LPCTSTR>(IDI_WINMAPVIEWER));
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = reinterpret_cast<LPCSTR>(IDC_WINMAPVIEWER);
		wcex.lpszClassName = TEXT("winmapviewer");
		wcex.hIconSm = LoadIcon(wcex.hInstance, reinterpret_cast<LPCTSTR>(IDI_SMALL));

		if (!RegisterClassEx(&wcex)) {
			throw "Error registering main window";
		}

		mainWindowIsRegistered = true;
	}

	m_hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		TEXT("winmapviewer"),
		TEXT("winmapviewer"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		NULL,
		NULL,
		m_hInstance,
		this
	);

	if (!m_hWnd) {
		std::cout << GetLastError() << std::endl;
		throw "Unable to create main window.";
	}

	mainWindowCount++;

	m_mapControl = new MapControl(m_hInstance, m_hWnd, m_tileCache);

	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	m_mapControl->create(0, 0, clientRect.right, clientRect.bottom);

	m_hwndStatusBar = CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		TEXT(""),
		m_hWnd,
		STATUS_BAR_CONTROL_IDENTIFIER
	);
	int partSizes[] = {100, 200, 260, -1};
	int numParts = sizeof(partSizes) / sizeof(partSizes[0]);
	SendMessage(m_hwndStatusBar, SB_SETPARTS, numParts, reinterpret_cast<LPARAM>(partSizes));

	updateStatusBarZoom();

	m_mapControl->setSettings(&m_settings);

	if (m_settings.styleIdentifier == IDM_STYLE_CUSTOM) {
		selectCustomStyle();
	} else {
		selectIntegratedStyle(m_settings.styleIdentifier);
	}

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return true;
}

LRESULT CALLBACK MainWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	int wmEvent;

	try {
		switch (message) {
			case WM_COMMAND:
				wmId = LOWORD(wParam);
				wmEvent = HIWORD(wParam);
				switch (wmId) {
					case IDM_ABOUT:
						DialogBox(m_hInstance, reinterpret_cast<LPCTSTR>(IDD_ABOUTBOX), m_hWnd, reinterpret_cast<DLGPROC>(MainWindow::aboutDialogWndProcStatic));
						break;

					case IDM_EXIT:
						DestroyWindow(m_hWnd);
						break;

					case IDM_SEARCH:
						// Freed by itself on WM_DESTORY
						new SearchDialog(m_hInstance, m_hWnd);
						break;

					case IDM_NEW_WINDOW: {
						m_mapControl->getSettings(&m_settings);
						// Freed by itself on WM_DESTORY
						MainWindow* newWindow = new MainWindow(m_hInstance, m_styleDatabase, m_settings, m_tileCache);
						newWindow->create(SW_SHOWNORMAL);
						break;
					}

					case IDM_ZOOMIN:
						m_mapControl->zoomIn();
						m_mapControl->getSettings(&m_settings);
						updateStatusBarZoom();
						m_mapControl->requestRedraw();
						break;

					case IDM_ZOOMOUT:
						m_mapControl->zoomOut();
						m_mapControl->getSettings(&m_settings);
						updateStatusBarZoom();
						m_mapControl->requestRedraw();
						break;

					case IDM_RIGHT:
						m_mapControl->setOffset(ARROW_KEYS_MOVE_DISTANCE, 0);
						m_mapControl->moveToOffset();
						m_mapControl->requestRedraw();
						break;

					case IDM_LEFT:
						m_mapControl->setOffset(-ARROW_KEYS_MOVE_DISTANCE, 0);
						m_mapControl->moveToOffset();
						m_mapControl->requestRedraw();
						break;

					case IDM_UP:
						m_mapControl->setOffset(0, -ARROW_KEYS_MOVE_DISTANCE);
						m_mapControl->moveToOffset();
						m_mapControl->requestRedraw();
						break;

					case IDM_DOWN:
						m_mapControl->setOffset(0, ARROW_KEYS_MOVE_DISTANCE);
						m_mapControl->moveToOffset();
						m_mapControl->requestRedraw();
						break;

					case IDM_STYLE_OSM_STANDARD:
					case IDM_STYLE_OSM_GERMAN:
					case IDM_STYLE_OEPNV:
					case IDM_STYLE_OPENTOPO:
						selectIntegratedStyle(wmId);
						break;

					case IDM_STYLE_CUSTOM: {
						int result = DialogBoxParam(
							m_hInstance,
							reinterpret_cast<LPCTSTR>(IDD_CUSTOMSTYLE),
							m_hWnd,
							reinterpret_cast<DLGPROC>(MainWindow::customStyleDialogWndProcStatic),
							reinterpret_cast<LPARAM>(this)
						);
						if (result == IDOK) {
							m_settings.styleIdentifier = IDM_STYLE_CUSTOM;
							selectCustomStyle();
						}
						break;
					}

					case IDM_USE_TLS:
						m_settings.useTls = !m_settings.useTls;
						if (m_settings.styleIdentifier != IDM_STYLE_CUSTOM) {
							selectIntegratedStyle(m_settings.styleIdentifier);
						} else {
							updateStyleMenu();
						}
						break;

					default:
						return DefWindowProc(m_hWnd, message, wParam, lParam);
				}
				break;

			case WM_ERASEBKGND:
				// reduce flickering
				return TRUE;

			case WM_SIZE: {
				SendMessage(m_hwndStatusBar, WM_SIZE, 0, 0);
				RECT rect;
				SendMessage(m_hwndStatusBar, SB_GETRECT, 0, reinterpret_cast<LPARAM>(&rect));
				MoveWindow(m_mapControl->m_hwndMap, 0, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) - (rect.bottom - rect.top + 2), TRUE);
				break;
			}

			case WM_NOTIFY: {
				LPNMHDR nm = reinterpret_cast<LPNMHDR>(lParam);
				if (nm->idFrom == STATUS_BAR_CONTROL_IDENTIFIER && nm->code == NM_CLICK) {
					LPNMMOUSE mouse = reinterpret_cast<LPNMMOUSE>(lParam);
					RECT rect;
					SendMessage(m_hwndStatusBar, SB_GETRECT, STATUS_BAR_PART_ATTRIBUTION, reinterpret_cast<LPARAM>(&rect));
					if (PtInRect(&rect, mouse->pt) && m_settings.styleIdentifier != IDM_STYLE_CUSTOM) {
						ShellExecute(NULL, "open", m_styleDatabase.get(m_settings.styleIdentifier)->attributionLink, NULL, NULL, SW_SHOWNORMAL);
					}
				}
				break;
			}

			case WM_MOUSEWHEEL:
				if (static_cast<short>(HIWORD(wParam)) > 0) {
					m_mapControl->zoomIn();
				} else {
					m_mapControl->zoomOut();
				}
				m_mapControl->requestRedraw();
				break;

			case WM_USER_MAP_LONLAT_UPDATE: {
				LonLat* updatedLonLat = reinterpret_cast<LonLat*>(lParam);
				char statusText[128];
				sprintf(statusText, TEXT("lon: %.6f"), updatedLonLat->lon);
				SendMessage(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_LON, reinterpret_cast<LPARAM>(statusText));
				sprintf(statusText, TEXT("lat: %.6f"), updatedLonLat->lat);
				SendMessage(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_LAT, reinterpret_cast<LPARAM>(statusText));
				break;
			}

			case WM_USER_SEARCH_SET_LONLAT: {
				m_mapControl->setCenterLonLat(reinterpret_cast<LonLat*>(lParam));
				m_mapControl->requestRedraw();
				break;
			}

			case WM_DESTROY: {
				m_mapControl->getSettings(&m_settings);
				storeSettingsInRegistry(m_settings);
				mainWindowCount--;
				if (mainWindowCount == 0) {
					PostQuitMessage(0);
				}
				// Not sure if this will break something :)
				// Would be bad if the window continues to receive messages.
				// TODO: delete it on WM_NCDESTROY?
				delete this;
				break;
			}

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} catch (char const* e) {
		MessageBox(NULL, e, TEXT("winmapviewer"), MB_OK);
		std::cerr << "Exception caught in main window procedure: " << e << std::endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}

LRESULT CALLBACK MainWindow::aboutDialogWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
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

LRESULT CALLBACK MainWindow::customStyleDialogWndProcStatic(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
			// Store pointer to MainWindow instance
			SetWindowLong(hwndDialog, GWL_USERDATA, lParam);
			return TRUE;

		case WM_COMMAND:
			MainWindow* mainWindow = reinterpret_cast<MainWindow*>(GetWindowLong(hwndDialog, GWL_USERDATA));
			if (LOWORD(wParam) == IDOK) {
				HWND hInputField = GetDlgItem(hwndDialog, IDC_DLG_TEXT);
				int length = GetWindowTextLength(hInputField) + 1;
				std::string customStyleUrlTemplate;
				customStyleUrlTemplate.resize(length);
				GetWindowTextA(hInputField, &customStyleUrlTemplate[0], length);

				TileKey dummyTileKey(customStyleUrlTemplate, 0, 0, 0);
				try {
					parseStyleUrlTemplate(dummyTileKey);
				} catch (const char* e) {
					MessageBox(NULL, e, TEXT("Invalid URL template"), MB_OK);
					return TRUE;
				}

				mainWindow->m_settings.customStyleUrlTemplate = customStyleUrlTemplate;
				EndDialog(hwndDialog, LOWORD(wParam));

				return TRUE;
			}
			if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDialog, LOWORD(wParam));
				return TRUE;
			}
			break;
	}

	return FALSE;
}

void MainWindow::selectIntegratedStyle(int styleIdentifier) {
	const Style* style = m_styleDatabase.get(styleIdentifier);
	m_settings.styleIdentifier = styleIdentifier;
	std::string styleUrlTemplate(m_settings.useTls ? style->url : style->urlInsecure);
	m_mapControl->setStyle(styleUrlTemplate);
	m_mapControl->setMaxZoomLevel(style->maxZoomLevel);

	do {
		m_mapControl->getSettings(&m_settings);
		if (m_settings.zoomLevel <= style->maxZoomLevel) {
			break;
		}
		m_mapControl->zoomOut();
	} while (true);
	updateStatusBarZoom();

	m_mapControl->requestRedraw();
	SendMessage(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ATTRIBUTION, reinterpret_cast<LPARAM>(TEXT(style->attributionText)));
	updateStyleMenu();
}

void MainWindow::selectCustomStyle() {
	m_mapControl->setStyle(m_settings.customStyleUrlTemplate);
	m_mapControl->setMaxZoomLevel(CUSTOM_STYLE_MAX_ZOOM_LEVEL);
	m_mapControl->requestRedraw();
	SendMessage(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ATTRIBUTION, reinterpret_cast<LPARAM>(TEXT("Custom map style")));
	updateStyleMenu();
}

void MainWindow::updateStyleMenu() {
	HMENU hMenu = GetMenu(m_hWnd);
	CheckMenuItem(hMenu, IDM_USE_TLS, m_settings.useTls ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuRadioItem(
		hMenu,
		IDM_STYLE_OSM_STANDARD,
		IDM_STYLE_CUSTOM,
		m_settings.styleIdentifier,
		MF_BYCOMMAND
	);
}

void MainWindow::updateStatusBarZoom() {
	char statusText[16];
	sprintf(statusText, TEXT("Zoom: %i"), m_settings.zoomLevel);
	SendMessage(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ZOOM, reinterpret_cast<LPARAM>(statusText));
}