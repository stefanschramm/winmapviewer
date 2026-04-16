#include "MainWindowManager.h"
#include "MainWindow.h"

MainWindowManager::MainWindowManager(
	const MapPrinter& mapPrinter,
	const SearchProvider& searchProvider,
	const StyleDatabase& styleDatabase,
	TileCache& tileCache,
	HINSTANCE hInstance
)
	: m_mapPrinter(mapPrinter),
	  m_searchProvider(searchProvider),
	  m_styleDatabase(styleDatabase),
	  m_tileCache(tileCache),
	  m_hInstance(hInstance),
	  m_windowCount(0) {
}

void MainWindowManager::create(Settings settings, int nCmdShow) {
	MainWindow* mainWindow = new MainWindow(
		*this,
		m_mapPrinter,
		m_searchProvider,
		m_styleDatabase,
		m_tileCache,
		m_hInstance,
		settings
	);
	mainWindow->create(nCmdShow);
	m_windowCount++;
}

void MainWindowManager::destroy(MainWindow* mainWindow) {
	delete mainWindow;
	m_windowCount--;
	if (m_windowCount == 0) {
		PostQuitMessage(0);
	}
}
