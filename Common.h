#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <string>
#include <windows.h>

// Map control notifies parent window about current cursor position
const int WM_USER_MAP_LONLAT_UPDATE = WM_USER + 1;

// Tile downloader notifies main thread about a new available tile
const int WM_USER_TILE_DOWNLOAD_FINISHED = WM_USER + 2;

// Tile cache notifies specific window that a tile can now be rendered
const int WM_USER_TILE_AVAILABLE = WM_USER + 3;

// Search dialog notifies main window that a search result was selected and the map should be centered to it
const int WM_USER_SEARCH_SET_LONLAT = WM_USER + 4;

struct LonLat {
	double lon;
	double lat;
};

HBITMAP createPlaceholderBitmap(bool error);

std::string urlEncode(const std::wstring& url);

// Template for wrapper that calls the wndProc instance method of the object corresponding to the window.
// It needs to be in a class because VC++ 6 can't use a function template directly.
template <class T>
class WndProcStaticHelper {
  public:
	static LRESULT CALLBACK wndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
	};
};
