#pragma once

#include <windows.h>

#include "StyleDatabase.h"
#include "resource.h"

class MainWindow {
  public:
	MainWindow(
		HINSTANCE hInstance,
		int nCmdShow,
		StyleDatabase& styleDatabase,
		int initialStyleIdentifier
	);

	static LRESULT CALLBACK wndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK customStyleDialogWndProcStatic(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK aboutDialogWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

  private:
	LRESULT CALLBACK wndProc(UINT message, WPARAM wParam, LPARAM lParam);
	void changeStyle(int styleIdentifier);

	static void registerWindow(HINSTANCE hInstance);

	static bool mainWindowIsRegistered;
	static int mainWindowCount;

	StyleDatabase& m_styleDatabase;
	// TODO: Put into settings struct that can be loaded/stored from/to registry
	int m_currentStyleIdentifier;
	bool m_useTls;

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HWND m_hwndStatusBar;
	HWND m_hwndMap;
};
