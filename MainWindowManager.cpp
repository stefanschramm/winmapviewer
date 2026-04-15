#include "MainWindowManager.h"
#include "MainWindow.h"

MainWindowManager::MainWindowManager(HINSTANCE hInstance, const StyleDatabase& styleDatabase, TileCache& tileCache, const SearchProvider& searchProvider)
	: m_styleDatabase(styleDatabase),
	  m_tileCache(tileCache),
	  m_searchProvider(searchProvider),
	  m_windowCount(0),
	  m_hInstance(hInstance) {
}

void MainWindowManager::create(Settings settings, int nCmdShow) {
	MainWindow* mainWindow = new MainWindow(m_hInstance, *this, m_styleDatabase, settings, m_tileCache, m_searchProvider);
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
