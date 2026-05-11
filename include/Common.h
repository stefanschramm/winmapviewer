#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <string>
#include <windows.h>

// VC++ 6 compatibility
#if _MSC_VER == 1200
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
#endif

#ifndef GWLP_USERDATA
#define GWLP_USERDATA GWL_USERDATA
#endif

#ifndef SetWindowLongPtr
#define SetWindowLongPtr(hwnd, index, value) SetWindowLong(hwnd, index, (LONG)(value))
#define GetWindowLongPtr(hwnd, index) GetWindowLong(hwnd, index)
#endif

#include "TileKey.h"

// Map control notifies parent window about current cursor position
const int WM_USER_MAP_LONLAT_UPDATE = WM_USER + 1;

// Tile downloader notifies main thread about a new available tile
const int WM_USER_TILE_DOWNLOAD_FINISHED = WM_USER + 2;

// Tile cache notifies specific window that a tile can now be rendered
const int WM_USER_TILE_AVAILABLE = WM_USER + 3;

// Search dialog notifies main window that a search result was selected and the map should be centered to it
const int WM_USER_SEARCH_SET_LONLAT = WM_USER + 4;

// Map control notifies parent window about new map position (when dragging finishes)
const int WM_USER_MAP_POSITION_UPDATED = WM_USER + 5;

struct LonLat {
	double lon;
	double lat;
};

std::string parseStyleUrlTemplate(const TileKey& tileKey);

void warningMessage(const char* message);

void panicMessage(const char* place, const char* message);

// Template for wrapper that calls the wndProc instance method of the object corresponding to the window.
// VC++ 6 compatibility: It needs to be in a class because VC++ 6 can't use a function template directly.
template <class T>
class WndProcStaticHelper {
  public:
	static LRESULT CALLBACK wndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		T* self = NULL;

		if (message == WM_NCCREATE) {
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			self = reinterpret_cast<T*>(cs->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
		} else {
			self = reinterpret_cast<T*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (!self) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (message == WM_NCDESTROY) {
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		}

		return self->wndProc(hWnd, message, wParam, lParam);
	};
};
