#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <map>
#include <queue>

#include "TileDownloader.h"
#include "TileKey.h"

const UINT WM_USER_TILE_READY = WM_USER + 1;

class DownloadWorker {
  public:
	DownloadWorker(TileDownloader* tileDownloader, HWND hwndMain);
	~DownloadWorker();
	void download(TileKey tileKey);
	void transferFinishedDownloads(std::map<TileKey, HBITMAP>* pCacheMap);

  private:
	std::queue<TileKey> m_downloadQueue;
	std::map<TileKey, HBITMAP> m_finishedDownloads;
	CRITICAL_SECTION m_mutex;
	HANDLE m_thread;
	DWORD m_threadId;
	HWND m_hwndMain;
	TileDownloader* m_tileDownloader;

	static DWORD WINAPI threadEntry(LPVOID lpParam) {
		DownloadWorker* worker = static_cast<DownloadWorker*>(lpParam);
		worker->run();
		return 0;
	}

	void run();
};
