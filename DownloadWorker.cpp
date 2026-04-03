#include <cstdlib>
#include <iostream>

#include "DownloadWorker.h"

DownloadWorker::DownloadWorker(const TileDownloader& tileDownloader, DWORD uiThreadId) : m_tileDownloader(tileDownloader), m_uiThreadId(uiThreadId) {
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

				HBITMAP hBitmap = m_tileDownloader.get(tileKey);

				EnterCriticalSection(&m_mutex);
				m_finishedDownloads[tileKey] = hBitmap;
				LeaveCriticalSection(&m_mutex);

				// TODO: defined WM_APP + 1 somewhere
				PostThreadMessage(m_uiThreadId, WM_APP + 1, 0, 0);
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
		exit(EXIT_FAILURE);
	}
}

void DownloadWorker::download(const TileKey& tileKey) {
	// called from the main thread
	EnterCriticalSection(&m_mutex);
	m_queuedDownloads.push_back(tileKey);
	LeaveCriticalSection(&m_mutex);
	if (ResumeThread(m_thread) == 0xFFFFFFFF) {
		// Disabling because for some reason Windows 95 fails this check all the time...
		// throw "Unable to resume worker thread.";
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
