#include <iostream>

#include "Common.h"
#include "HiddenWindow.h"

// VC++ 6 compatibility
#ifndef HWND_MESSAGE
#define HWND_MESSAGE ((HWND) - 3)
#endif

HiddenWindow::HiddenWindow(HINSTANCE hInstance, TileCache& tileCache) : m_hInstance(hInstance), m_tileCache(tileCache) {
}

HWND HiddenWindow::create() {
	static bool windowIsRegistered = false;
	static WNDCLASS wc;

	if (!windowIsRegistered) {
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = &WndProcStaticHelper<HiddenWindow>::wndProcStatic;
		wc.lpszClassName = "HiddenMessageWindow";
		RegisterClass(&wc);
		windowIsRegistered = true;
	}

	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		NULL,
		WS_POPUP,
		0, 0, 0, 0,
		NULL,
		NULL,
		m_hInstance,
		this
	);

	if (!hwnd) {
		std::cout << GetLastError() << std::endl;
		throw "Unable to create hidden window.";
	}

	return hwnd;
}

LRESULT CALLBACK HiddenWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const {
	if (message == WM_USER_TILE_DOWNLOAD_FINISHED) {
		m_tileCache.onDownloadFinished();
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
