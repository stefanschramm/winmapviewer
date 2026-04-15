// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <cstdlib>

#include "HiddenWindow.h"
#include "MainWindow.h"
#include "MainWindowManager.h"
#include "SearchProvider.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

#define MAX_LOADSTRING 100

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	try {
		InitCommonControls();

		const StyleDatabase styleDatabase(IDM_STYLE_OSM_STANDARD);
		const GdiPlusWrapper gdiPlusWrapper;
		const TileDownloader tileDownloader(gdiPlusWrapper);
		DownloadWorker downloadWorker(tileDownloader);
		TileCache tileCache(downloadWorker);
		HiddenWindow hiddenWindow(hInstance, tileCache);
		downloadWorker.setNotificationReceiver(hiddenWindow.create());
		SearchProvider searchProvider;
		MainWindowManager mainWindowManager(hInstance, styleDatabase, tileCache, searchProvider);

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
