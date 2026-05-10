#pragma once

#include <vector>

#include "GpxLoader.h"
#include "MapPrinter.h"
#include "SearchProvider.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "TileCache.h"

// Forward declaration (because of cyclic dependency)
class MainWindow;

class MainWindowManager {
  public:
	MainWindowManager(
		const GpxLoader& gpxLoader,
		const MapPrinter& mapPrinter,
		const SearchProvider& searchProvider,
		const StyleDatabase& styleDatabase,
		TileCache& tileCache,
		HINSTANCE hInstance
	);
	void create(Settings settings, int nCmdShow);
	void destroy(MainWindow* mainWindow);
	void setCenterLonLat(LonLat* lonLat, MainWindow* triggeringMainWindow);

  private:
	const GpxLoader& m_gpxLoader;
	const MapPrinter& m_mapPrinter;
	const SearchProvider& m_searchProvider;
	const StyleDatabase& m_styleDatabase;
	TileCache& m_tileCache;

	HINSTANCE m_hInstance;
	std::vector<MainWindow*> m_windows;
};
