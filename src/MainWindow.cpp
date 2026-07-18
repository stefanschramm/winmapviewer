// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <windowsx.h>

#include "Common.h"
#include "MainWindow.h"
#include "MapPrinter.h"
#include "SearchDialog.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

// VC++ 6 compatibility
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

MainWindow::MainWindow(
	const GpxLoader& gpxLoader,
	MainWindowManager& mainWindowManager,
	const MapPrinter& mapPrinter,
	const SearchProvider& searchProvider,
	const StyleDatabase& styleDatabase,
	TileCache& tileCache,
	HINSTANCE hInstance,
	Settings settings
) : m_gpxLoader(gpxLoader),
	m_mainWindowManager(mainWindowManager),
	m_mapPrinter(mapPrinter),
	m_searchProvider(searchProvider),
	m_styleDatabase(styleDatabase),
	m_tileCache(tileCache),
	m_hInstance(hInstance),
	m_settings(settings),
	m_maxZoomLevel(CUSTOM_STYLE_MAX_ZOOM_LEVEL) {
}

bool MainWindow::create(int nCmdShow) {
	createMainWindow();

	createMapControl();

	createStatusBar();
	updateStatusBarZoom();

	applyInitialSettings();

	DragAcceptFiles(m_hWnd, true);

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	return true;
}

void MainWindow::createMainWindow() {
	static bool mainWindowIsRegistered = false;

	if (!mainWindowIsRegistered) {
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &WndProcStaticHelper<MainWindow>::wndProcStatic;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = LoadIcon(wcex.hInstance, reinterpret_cast<LPCTSTR>(IDI_WINMAPVIEWER));
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
		throw "Unable to create main window.";
	}
}

void MainWindow::createMapControl() {
	m_mapControl = new MapControl(m_hInstance, m_hWnd, m_tileCache);

	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	m_mapControl->create(0, 0, clientRect.right, clientRect.bottom);
}

void MainWindow::createStatusBar() {
	m_hwndStatusBar = CreateStatusWindow(
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		TEXT(""),
		m_hWnd,
		STATUS_BAR_CONTROL_IDENTIFIER
	);
	int partSizes[] = {100, 200, 260, -1};
	int numParts = sizeof(partSizes) / sizeof(partSizes[0]);
	SendMessage(m_hwndStatusBar, SB_SETPARTS, numParts, reinterpret_cast<LPARAM>(partSizes));
}

void MainWindow::applyInitialSettings() {
	m_mapControl->setSettings(&m_settings);

	if (m_settings.styleIdentifier == IDM_STYLE_CUSTOM) {
		selectCustomStyle();
	} else {
		selectIntegratedStyle(m_settings.styleIdentifier);
	}

	updateSyncPositionMenuEntry();
}

LRESULT CALLBACK MainWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	try {
		switch (message) {
			case WM_COMMAND:
				wmId = LOWORD(wParam);
				switch (wmId) {
					case IDM_ABOUT:
						DialogBox(m_hInstance, reinterpret_cast<LPCTSTR>(IDD_ABOUTBOX), m_hWnd, reinterpret_cast<DLGPROC>(MainWindow::aboutDialogWndProcStatic));
						break;

					case IDM_EXIT:
						DestroyWindow(m_hWnd);
						break;

					case IDM_SEARCH: {
						SearchDialog searchDialog(m_hInstance, m_hWnd, m_searchProvider);
						searchDialog.show();
						break;
					}

					case IDM_NEW_WINDOW: {
						m_mapControl->getSettings(&m_settings);
						m_mainWindowManager.create(m_settings, SW_SHOWNORMAL);
						break;
					}

					case IDM_ZOOMIN:
						zoom(1);
						break;

					case IDM_ZOOMOUT:
						zoom(-1);
						break;

					case IDM_RIGHT:
					case IDM_LEFT:
					case IDM_UP:
					case IDM_DOWN:
						move(wmId);
						break;

					case IDM_STYLE_OSM_STANDARD:
					case IDM_STYLE_OSM_GERMAN:
					case IDM_STYLE_OEPNV:
					case IDM_STYLE_OPENTOPO:
					case IDM_STYLE_SWISS:
						selectIntegratedStyle(wmId);
						break;

					case IDM_STYLE_CUSTOM:
						showCustomStyleDialog();
						break;

					case IDM_USE_TLS:
						toggleTls();
						break;

					case IDM_SYNC_POSITION:
						m_settings.syncPosition = !m_settings.syncPosition;
						updateSyncPositionMenuEntry();
						syncOtherWindowsPositions();
						break;

					case IDM_PRINT:
						print();
						break;

					case IDM_LOAD_TRACK:
						loadTrack();
						break;

					case IDM_CLEAR_TRACK:
						m_mapControl->clearTracks();
						m_mapControl->requestRedraw();
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
				// Check if notification was a click at the attribution text part of the status bar
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
				zoomByMouseWheel(static_cast<short>(HIWORD(wParam)) > 0 ? 1 : -1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				break;

			case WM_DROPFILES:
				onDropFiles(reinterpret_cast<HDROP>(wParam));
				break;

			case WM_USER_MAP_LONLAT_UPDATE:
				onLonLatUpdate(reinterpret_cast<LonLat*>(lParam));
				break;

			case WM_USER_SEARCH_SET_LONLAT:
				setCenterLonLat(reinterpret_cast<LonLat*>(lParam));
				break;

			case WM_USER_MAP_POSITION_UPDATED:
				syncOtherWindowsPositions();
				break;

			case WM_DESTROY:
				m_mapControl->getSettings(&m_settings);
				storeSettingsInRegistry(m_settings);
				break;

			case WM_NCDESTROY:
				m_mainWindowManager.destroy(this);
				break;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} catch (char const* e) {
		panicMessage("main window procedure", e);
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
		case WM_INITDIALOG: {
			// Store pointer to MainWindow instance
			SetWindowLongPtr(hwndDialog, GWLP_USERDATA, lParam);

			MainWindow* mainWindow = reinterpret_cast<MainWindow*>(lParam);

			HWND hInputField = GetDlgItem(hwndDialog, IDC_DLG_TEXT);
			SetWindowTextA(hInputField, mainWindow->m_settings.customStyleUrlTemplate.c_str());

			return TRUE;
		}

		case WM_COMMAND:
			MainWindow* mainWindow = reinterpret_cast<MainWindow*>(GetWindowLong(hwndDialog, GWLP_USERDATA));
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
					MessageBoxA(NULL, e, "Invalid URL template", MB_OK | MB_ICONWARNING);
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

void MainWindow::setCenterLonLat(LonLat* lonLat) {
	m_mapControl->setCenterLonLat(lonLat);
	m_mapControl->requestRedraw();
}

void MainWindow::zoom(int zoomLevelDelta) {
	int newZoomLevel = m_settings.zoomLevel + zoomLevelDelta;
	if (newZoomLevel > m_maxZoomLevel || newZoomLevel < 0) {
		return;
	}
	m_mapControl->setZoomLevel(newZoomLevel);
	m_mapControl->getSettings(&m_settings);
	updateStatusBarZoom();
	m_mapControl->requestRedraw();
}

void MainWindow::zoomByMouseWheel(int zoomLevelDelta, int x, int y) {
	int newZoomLevel = m_settings.zoomLevel + zoomLevelDelta;
	if (newZoomLevel > m_maxZoomLevel || newZoomLevel < 0) {
		return;
	}
	POINT pt = {x, y};
	ScreenToClient(m_mapControl->m_hwndMap, &pt);
	m_mapControl->setZoomLevelKeepingFixPoint(newZoomLevel, pt.x, pt.y);
	m_mapControl->getSettings(&m_settings);
	updateStatusBarZoom();
	m_mapControl->requestRedraw();
}

void MainWindow::move(int direction) {
	switch (direction) {
		case IDM_UP:
			m_mapControl->setOffset(0, -ARROW_KEYS_MOVE_DISTANCE);
			break;
		case IDM_RIGHT:
			m_mapControl->setOffset(ARROW_KEYS_MOVE_DISTANCE, 0);
			break;
		case IDM_DOWN:
			m_mapControl->setOffset(0, +ARROW_KEYS_MOVE_DISTANCE);
			break;
		case IDM_LEFT:
			m_mapControl->setOffset(-ARROW_KEYS_MOVE_DISTANCE, 0);
			break;
		default:
			throw "Unexpected move direction";
	}
	m_mapControl->moveToOffset();
	m_mapControl->requestRedraw();
}

void MainWindow::toggleTls() {
	m_settings.useTls = !m_settings.useTls;
	if (m_settings.styleIdentifier != IDM_STYLE_CUSTOM) {
		selectIntegratedStyle(m_settings.styleIdentifier);
	} else {
		updateStyleMenu();
	}
}

void MainWindow::showCustomStyleDialog() {
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
}

void MainWindow::selectIntegratedStyle(int styleIdentifier) {
	const Style* style = m_styleDatabase.get(styleIdentifier);
	m_settings.styleIdentifier = styleIdentifier;
	std::string styleUrlTemplate(m_settings.useTls ? style->url : style->urlInsecure);
	m_mapControl->setStyle(styleUrlTemplate);
	m_maxZoomLevel = style->maxZoomLevel;
	if (m_settings.zoomLevel > style->maxZoomLevel) {
		m_mapControl->setZoomLevel(style->maxZoomLevel);
		// update x, y and zoomLevel
		m_mapControl->getSettings(&m_settings);
		updateStatusBarZoom();
	}

	m_mapControl->requestRedraw();
	SendMessageA(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ATTRIBUTION, reinterpret_cast<LPARAM>(style->attributionText));
	updateStyleMenu();
}

void MainWindow::selectCustomStyle() {
	m_mapControl->setStyle(m_settings.customStyleUrlTemplate);
	m_maxZoomLevel = CUSTOM_STYLE_MAX_ZOOM_LEVEL;
	m_mapControl->requestRedraw();
	SendMessageA(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ATTRIBUTION, reinterpret_cast<LPARAM>("Custom map style"));
	updateStyleMenu();
}

void MainWindow::onLonLatUpdate(LonLat* updatedLonLat) {
	char statusText[128];
	sprintf(statusText, "lon: %.6f", updatedLonLat->lon);
	SendMessageA(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_LON, reinterpret_cast<LPARAM>(statusText));
	sprintf(statusText, "lat: %.6f", updatedLonLat->lat);
	SendMessageA(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_LAT, reinterpret_cast<LPARAM>(statusText));
}

void MainWindow::onDropFiles(HDROP hDrop) {
	unsigned int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for (unsigned int i = 0; i < count; i++) {
		TCHAR filePath[MAX_PATH];
		DragQueryFile(hDrop, i, filePath, MAX_PATH);
		loadGpxFile(filePath);
	}
	m_mapControl->requestRedraw();
}

void MainWindow::loadGpxFile(const char* filePath) {
	std::vector<std::vector<LonLat> > tracks = m_gpxLoader.load(filePath);
	for (std::vector<std::vector<LonLat> >::iterator iterator = tracks.begin(); iterator != tracks.end(); iterator++) {
		m_mapControl->addTrack(*iterator);
	}
}

void MainWindow::syncOtherWindowsPositions() {
	if (m_settings.syncPosition) {
		LonLat lonLat;
		m_mapControl->getCenterLonLat(&lonLat);
		m_mainWindowManager.setCenterLonLat(&lonLat, this);
	}
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

void MainWindow::updateSyncPositionMenuEntry() {
	HMENU hMenu = GetMenu(m_hWnd);
	CheckMenuItem(hMenu, IDM_SYNC_POSITION, m_settings.syncPosition ? MF_CHECKED : MF_UNCHECKED);
}

void MainWindow::updateStatusBarZoom() {
	char statusText[16];
	sprintf(statusText, "Zoom: %i", m_settings.zoomLevel);
	SendMessageA(m_hwndStatusBar, SB_SETTEXT, STATUS_BAR_PART_ZOOM, reinterpret_cast<LPARAM>(statusText));
}

void MainWindow::print() {
	HMENU hMenu = GetMenu(m_hWnd);
	EnableMenuItem(hMenu, IDM_PRINT, MF_GRAYED);

	m_mapControl->getSettings(&m_settings);

	std::string styleUrlTemplate;
	if (m_settings.styleIdentifier == IDM_STYLE_CUSTOM) {
		styleUrlTemplate = m_settings.customStyleUrlTemplate;
	} else {
		const Style* style = m_styleDatabase.get(m_settings.styleIdentifier);
		styleUrlTemplate = m_settings.useTls ? style->url : style->urlInsecure;
	}

	m_mapPrinter.print(
		m_hWnd,
		m_settings.zoomLevel,
		m_settings.centerX,
		m_settings.centerY,
		styleUrlTemplate
	);

	EnableMenuItem(hMenu, IDM_PRINT, MF_ENABLED);
}

void MainWindow::loadTrack() {
	char fileName[MAX_PATH] = {0};

	OPENFILENAMEA ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = "GPX Tracks\0*.gpx\0";
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = "gpx";

	if (GetOpenFileNameA(&ofn)) {
		loadGpxFile(fileName);
		m_mapControl->requestRedraw();
	}
}
