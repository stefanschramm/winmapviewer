#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <map>
#include <queue>

#include "TileDownloader.h"
#include "TileKey.h"

class DownloadWorker {
  public:
	DownloadWorker(const TileDownloader& tileDownloader, DWORD uiThreadId);
	~DownloadWorker();
	void download(const TileKey& tileKey);
	void transferFinishedDownloads(std::map<TileKey, HBITMAP>* pCacheMap);
	void unqueue(const TileKey& tileKey);

  private:
	std::deque<TileKey> m_queuedDownloads;
	std::map<TileKey, HBITMAP> m_finishedDownloads;
	CRITICAL_SECTION m_mutex;
	HANDLE m_thread;
	DWORD m_threadId;
	DWORD m_uiThreadId;
	const TileDownloader& m_tileDownloader;

	static DWORD WINAPI threadEntry(LPVOID lpParam) {
		DownloadWorker* worker = static_cast<DownloadWorker*>(lpParam);
		worker->run();
		return 0;
	}

	void run();
};
