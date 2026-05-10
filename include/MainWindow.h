#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <windows.h>

#include "MainWindowManager.h"
#include "MapControl.h"
#include "MapPrinter.h"
#include "SearchProvider.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "resource.h"

class MainWindow {
  public:
	MainWindow(
		const GpxLoader& gpxLoader,
		MainWindowManager& mainWindowManager,
		const MapPrinter& mapPrinter,
		const SearchProvider& searchProvider,
		const StyleDatabase& styleDatabase,
		TileCache& tileCache,
		HINSTANCE hInstance,
		Settings settings
	);
	bool create(int nCmdShow);
	static LRESULT CALLBACK customStyleDialogWndProcStatic(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK aboutDialogWndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void setCenterLonLat(LonLat* lonLat);

  private:
	void createMainWindow();
	void createMapControl();
	void createStatusBar();
	void applyInitialSettings();

	void zoom(int zoomLevelDelta);
	void zoomByMouseWheel(int zoomLevelDelta, int x, int y);
	void move(int direction);
	void toggleTls();
	void showCustomStyleDialog();
	void selectIntegratedStyle(int styleIdentifier);
	void selectCustomStyle();
	void print();
	void loadTrack();

	void onLonLatUpdate(LonLat* updatedLonLat);
	void onDropFiles(HDROP hDrop);

	void syncOtherWindowsPositions();

	void updateStyleMenu();
	void updateSyncPositionMenuEntry();
	void updateStatusBarZoom();

	const GpxLoader& m_gpxLoader;
	const MapPrinter& m_mapPrinter;
	MainWindowManager& m_mainWindowManager;
	const SearchProvider& m_searchProvider;
	Settings m_settings;
	const StyleDatabase& m_styleDatabase;
	TileCache& m_tileCache;

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HWND m_hwndStatusBar;
	MapControl* m_mapControl;
	int m_maxZoomLevel;
};
