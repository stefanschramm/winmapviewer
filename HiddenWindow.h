#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <windows.h>

#include "TileCache.h"

/**
 * HiddenWindow only exists to be able to receive "global" messages when a download has finished
 * and notify the TileCache, that then notifies the specific window(s) that requested the tile.
 */
class HiddenWindow {
  public:
	HiddenWindow(HINSTANCE hInstance, TileCache& tileCache);
	HWND create();
	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const;

  private:
	HINSTANCE m_hInstance;
	TileCache& m_tileCache;
};
