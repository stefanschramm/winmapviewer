#include <cstdlib>
#include <iostream>

#include "Common.h"
#include "DownloadWorker.h"

DownloadWorker::DownloadWorker(const TileDownloader& tileDownloader) : m_tileDownloader(tileDownloader), m_hwndNotificationReceiver(0), m_stop(false) {
	InitializeCriticalSection(&m_mutex);

	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_event) {
		throw "Failed to create event.";
	}

	DWORD threadId;
	m_thread = CreateThread(NULL, 0, threadEntry, this, 0, &threadId);
	if (!m_thread) {
		CloseHandle(m_event);
		throw "Failed to create download worker thread.";
	}
}

DownloadWorker::~DownloadWorker() {
	EnterCriticalSection(&m_mutex);
	m_stop = true;
	LeaveCriticalSection(&m_mutex);

	SetEvent(m_event);

	WaitForSingleObject(m_thread, INFINITE);
	CloseHandle(m_thread);
	CloseHandle(m_event);

	DeleteCriticalSection(&m_mutex);
}

void DownloadWorker::run() {
	try {
		while (true) {
			EnterCriticalSection(&m_mutex);
			if (m_stop) {
				LeaveCriticalSection(&m_mutex);
				break;
			}
			if (m_queuedDownloads.empty()) {
				LeaveCriticalSection(&m_mutex);
				if (WaitForSingleObject(m_event, INFINITE) == WAIT_FAILED) {
					throw "Failed to wait for event.";
				}
				continue;
			}
			TileKey tileKey = m_queuedDownloads.front();
			m_queuedDownloads.pop_front();
			LeaveCriticalSection(&m_mutex);

			HBITMAP hBitmap = m_tileDownloader.get(tileKey);

			EnterCriticalSection(&m_mutex);
			m_finishedDownloads[tileKey] = hBitmap;
			LeaveCriticalSection(&m_mutex);

			if (m_hwndNotificationReceiver != 0) {
				PostMessage(m_hwndNotificationReceiver, WM_USER_TILE_DOWNLOAD_FINISHED, 0, 0);
			}
		}
	} catch (char const* e) {
		MessageBox(NULL, e, TEXT("winmapviewer"), MB_OK);
		std::cerr << "Exception caught in download worker main loop: " << e << std::endl;
		exit(EXIT_FAILURE);
	}
}

void DownloadWorker::download(const TileKey& tileKey) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);
	m_queuedDownloads.push_back(tileKey);
	LeaveCriticalSection(&m_mutex);
	if (!SetEvent(m_event)) {
		throw "Failed to set event.";
	}
}

void DownloadWorker::transferFinishedDownloads(std::map<TileKey, HBITMAP>* pFinishedDownloads) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);

	for (std::map<TileKey, HBITMAP>::iterator it = m_finishedDownloads.begin(); it != m_finishedDownloads.end(); ++it) {
		(*pFinishedDownloads)[it->first] = it->second;
	}
	m_finishedDownloads.clear();

	LeaveCriticalSection(&m_mutex);
}

void DownloadWorker::unqueue(const TileKey& tileKey) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);

	std::deque<TileKey>::iterator it = m_queuedDownloads.begin();
	while (it != m_queuedDownloads.end()) {
		if (*it == tileKey) {
			it = m_queuedDownloads.erase(it);
		} else {
			++it;
		}
	}

	LeaveCriticalSection(&m_mutex);
}

void DownloadWorker::setNotificationReceiver(HWND hwndNotificationReceiver) {
	m_hwndNotificationReceiver = hwndNotificationReceiver;
}
