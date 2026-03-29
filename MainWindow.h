#pragma once

#include <windows.h>

#include "MapControl.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

class MainWindow {
  public:
	MainWindow(HINSTANCE hInstance, StyleDatabase& styleDatabase, Settings settings);
	bool create(int nCmdShow);
	static LRESULT CALLBACK wndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK customStyleDialogWndProcStatic(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK aboutDialogWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

  private:
	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void changeStyle(int styleIdentifier);

	static int mainWindowCount;

	StyleDatabase& m_styleDatabase;
	Settings m_settings;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HWND m_hwndStatusBar;
	MapControl* m_mapControl;
};
