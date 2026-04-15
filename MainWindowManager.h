#pragma once

#include "SearchProvider.h"
#include "Settings.h"
#include "StyleDatabase.h"
#include "TileCache.h"

// Forward declaration (because of cyclic dependency)
class MainWindow;

class MainWindowManager {
  public:
	MainWindowManager(HINSTANCE hInstance, const StyleDatabase& styleDatabase, TileCache& tileCache, const SearchProvider& searchProvider);
	void create(Settings settings, int nCmdShow);
	void destroy(MainWindow* mainWindow);

  private:
	const StyleDatabase& m_styleDatabase;
	TileCache& m_tileCache;
	const SearchProvider& m_searchProvider;
	int m_windowCount;
	HINSTANCE m_hInstance;
};
