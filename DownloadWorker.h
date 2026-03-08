#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <map>
#include <queue>

#include "TileDownloader.h"
#include "TileKey.h"
#include "TileRange.h"

const UINT WM_USER_TILE_READY = WM_USER + 1;

class DownloadWorker {
  public:
	DownloadWorker(const TileDownloader* tileDownloader, HWND hwndMain);
	~DownloadWorker();
	void download(TileKey tileKey);
	void transferFinishedDownloads(std::map<TileKey, HBITMAP>* pCacheMap);
	void unqueueInvisible(TileRange visibleTiles, std::map<TileKey, HBITMAP>* pCacheMap);

  private:
	std::deque<TileKey> m_queuedDownloads;
	std::map<TileKey, HBITMAP> m_finishedDownloads;
	CRITICAL_SECTION m_mutex;
	HANDLE m_thread;
	DWORD m_threadId;
	HWND m_hwndMain;
	const TileDownloader* const m_tileDownloader;

	static DWORD WINAPI threadEntry(LPVOID lpParam) {
		DownloadWorker* worker = static_cast<DownloadWorker*>(lpParam);
		worker->run();
		return 0;
	}

	void run();
};
