#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <windows.h>

#include "MapControl.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

class MainWindow {
  public:
	MainWindow(HINSTANCE hInstance, const StyleDatabase& styleDatabase, Settings settings, TileCache& tileCache);
	bool create(int nCmdShow);
	static LRESULT CALLBACK customStyleDialogWndProcStatic(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK aboutDialogWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
	void selectIntegratedStyle(int styleIdentifier);
	void selectCustomStyle();
	void updateStyleMenu();

	static int mainWindowCount;

	const StyleDatabase& m_styleDatabase;
	Settings m_settings;
	TileCache& m_tileCache;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HWND m_hwndStatusBar;
	MapControl* m_mapControl;
};
