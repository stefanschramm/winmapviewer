// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on

#include "GpxLoader.h"
#include "HiddenWindow.h"
#include "MainWindow.h"
#include "MainWindowManager.h"
#include "MapPrinter.h"
#include "SearchProvider.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "TileFetcher.h"
#include "resource.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	try {
		InitCommonControls();

		const GpxLoader gpxLoader;
		const StyleDatabase styleDatabase(IDM_STYLE_OSM_STANDARD);
		const TileFetcher tileFetcher;
		DownloadWorker downloadWorker(tileFetcher);
		TileCache tileCache(downloadWorker, tileFetcher);
		HiddenWindow hiddenWindow(hInstance, tileCache);
		downloadWorker.setNotificationReceiver(hiddenWindow.create());
		const SearchProvider searchProvider;
		const MapPrinter mapPrinter(tileCache);
		MainWindowManager mainWindowManager(gpxLoader, mapPrinter, searchProvider, styleDatabase, tileCache, hInstance);

		mainWindowManager.create(loadSettingsFromRegistry(), nCmdShow);

		HACCEL hAccelTable = LoadAccelerators(hInstance, reinterpret_cast<LPCTSTR>(IDC_WINMAPVIEWER));
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return msg.wParam;
	} catch (char const* e) {
		panicMessage("WinMain", e);
		exit(EXIT_FAILURE);
	}
}
