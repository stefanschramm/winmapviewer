#pragma once

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
		const MapPrinter& mapPrinter,
		const SearchProvider& searchProvider,
		const StyleDatabase& styleDatabase,
		TileCache& tileCache,
		HINSTANCE hInstance
	);
	void create(Settings settings, int nCmdShow);
	void destroy(MainWindow* mainWindow);

  private:
	const MapPrinter& m_mapPrinter;
	const SearchProvider& m_searchProvider;
	const StyleDatabase& m_styleDatabase;
	TileCache& m_tileCache;

	HINSTANCE m_hInstance;
	int m_windowCount;
};
