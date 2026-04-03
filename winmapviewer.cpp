// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <cstdlib>
#include <iostream>

#include "MainWindow.h"
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
		DownloadWorker downloadWorker(tileDownloader, GetCurrentThreadId());
		TileCache tileCache(downloadWorker);

		// Freed by itself on WM_DESTORY; Maybe creating some window management service would be useful
		MainWindow* mainWindow = new MainWindow(hInstance, styleDatabase, loadSettingsFromRegistry(), tileCache);
		mainWindow->create(nCmdShow);

		HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WINMAPVIEWER);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			// TODO: define constant for WM_APP + 1
			if (msg.message == WM_APP + 1 && msg.hwnd == 0) {
				tileCache.onDownloadFinished();
				continue;
			}
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return msg.wParam;
	} catch (char const* e) {
		MessageBox(NULL, e, TEXT("winmapviewer"), MB_OK);
		std::cerr << "Exception caught in WinMain: " << e << std::endl;
		exit(EXIT_FAILURE);
	}
}
