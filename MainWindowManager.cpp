#include <algorithm>
#include <iostream>

#include "MainWindow.h"
#include "MainWindowManager.h"

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
	  m_windows() {
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
	m_windows.push_back(mainWindow);
	mainWindow->create(nCmdShow);
}

void MainWindowManager::destroy(MainWindow* mainWindow) {
	m_windows.erase(std::remove(m_windows.begin(), m_windows.end(), mainWindow));
	delete mainWindow;
	if (m_windows.size() == 0) {
		PostQuitMessage(0);
	}
}

void MainWindowManager::setCenterLonLat(LonLat* lonLat, MainWindow* triggeringMainWindow) {
	for (std::vector<MainWindow*>::iterator it = m_windows.begin(); it != m_windows.end(); ++it) {
		if (*it == triggeringMainWindow) {
			continue; // don't send to self
		}
		(*it)->setCenterLonLat(lonLat);
	}
}
