#pragma once

#include <string>
#include <windows.h>

struct LonLat {
	double lon;
	double lat;
};

HBITMAP createPlaceholderBitmap(bool error);

std::string urlEncode(const std::wstring& url);

// Template for wrapper that calls the wndProc instance method of the object corresponding to the window.
template <class T>
LRESULT CALLBACK wndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	T* self = NULL;

	if (message == WM_NCCREATE) {
		CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		self = reinterpret_cast<T*>(cs->lpCreateParams);
		SetWindowLong(hWnd, GWL_USERDATA, reinterpret_cast<LONG>(self));
	} else {
		self = reinterpret_cast<T*>(GetWindowLong(hWnd, GWL_USERDATA));
	}

	if (!self) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	if (message == WM_NCDESTROY) {
		SetWindowLong(hWnd, GWL_USERDATA, 0);
	}

	return self->wndProc(hWnd, message, wParam, lParam);
}
