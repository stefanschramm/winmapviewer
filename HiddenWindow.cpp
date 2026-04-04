#include <iostream>

#include "Common.h"
#include "HiddenWindow.h"

HiddenWindow::HiddenWindow(HINSTANCE hInstance, TileCache& tileCache) : m_hInstance(hInstance), m_tileCache(tileCache) {
}

HWND HiddenWindow::create() {
	static bool windowIsRegistered = false;
	static WNDCLASS wc;

	if (!windowIsRegistered) {
		wc = {};
		wc.lpfnWndProc = wndProcStatic<HiddenWindow>;
		wc.lpszClassName = "HiddenMessageWindow";
		RegisterClass(&wc);
	}

	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		NULL,
		0, 0, 0, 0, 0,
		HWND_MESSAGE, // message-only window
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
	// TODO: Define WM_USER + 23 somewhere
	if (message == WM_USER + 23) {
		m_tileCache.onDownloadFinished();
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
