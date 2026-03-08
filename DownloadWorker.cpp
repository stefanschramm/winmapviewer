#include <cstdlib>

#include "DownloadWorker.h"

DownloadWorker::DownloadWorker(const TileDownloader* tileDownloader, HWND hwndMain) : m_tileDownloader(tileDownloader), m_hwndMain(hwndMain) {
	InitializeCriticalSection(&m_mutex);

	m_thread = CreateThread(NULL, 0, threadEntry, this, 0, &m_threadId);
	if (!m_thread) {
		throw "Failed to create download worker thread.";
	}
}

DownloadWorker::~DownloadWorker() {
	TerminateThread(m_thread, 0);

	DeleteCriticalSection(&m_mutex);
}

void DownloadWorker::run() {
	try {
		while (true) {
			if (!m_queuedDownloads.empty()) {
				EnterCriticalSection(&m_mutex);
				TileKey tileKey = m_queuedDownloads.front();
				m_queuedDownloads.pop_front();
				LeaveCriticalSection(&m_mutex);

				HBITMAP hBitmap = m_tileDownloader->get(tileKey);

				EnterCriticalSection(&m_mutex);
				m_finishedDownloads[tileKey] = hBitmap;
				LeaveCriticalSection(&m_mutex);

				PostMessage(m_hwndMain, WM_USER_TILE_READY, 0, 0);
			}
			if (m_queuedDownloads.empty()) {
				// wait for further download requests
				if (SuspendThread(m_thread) == 0xFFFFFFFF) {
					throw "Unable to suspend myself.";
				}
			}
		}
	} catch (char const* e) {
		MessageBox(NULL, e, TEXT("winmapviewer"), MB_OK);
		std::cerr << "Exception caught in download worker main loop: " << e << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void DownloadWorker::download(TileKey tileKey) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);
	m_queuedDownloads.push_back(tileKey);
	LeaveCriticalSection(&m_mutex);
	if (ResumeThread(m_thread) == 0xFFFFFFFF) {
		// Disabling because for some reason Windows 95 fails this check all the time...
		// throw "Unable to resume worker thread.";
	}
}

void DownloadWorker::transferFinishedDownloads(std::map<TileKey, HBITMAP>* pCacheMap) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);

	for (std::map<TileKey, HBITMAP>::iterator it = m_finishedDownloads.begin(); it != m_finishedDownloads.end(); ++it) {
		(*pCacheMap)[it->first] = it->second;
	}
	m_finishedDownloads.clear();

	LeaveCriticalSection(&m_mutex);
}

void DownloadWorker::unqueueInvisible(TileRange visibleTiles, std::map<TileKey, HBITMAP>* pCacheMap) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);

	std::deque<TileKey>::iterator it = m_queuedDownloads.begin();
	while (it != m_queuedDownloads.end()) {
		if (!visibleTiles.contains(*it)) {
			pCacheMap->erase(*it); // remove placeholder image reference
			it = m_queuedDownloads.erase(it);
		} else {
			++it;
		}
	}

	LeaveCriticalSection(&m_mutex);
}
