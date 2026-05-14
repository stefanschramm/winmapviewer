#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <map>

#include "DownloadWorker.h"
#include "TileKey.h"
#include "TileRange.h"

typedef std::vector<HWND> Subscribers;

struct CacheContent {
	bool available;
	HBITMAP bitmap;
	Subscribers subscribers;
	DWORD lastAccess;
};

class TileCache {
  public:
	TileCache(DownloadWorker& downloadWorker, const TileFetcher& tileFetcher);
	~TileCache();
	HBITMAP get(const TileKey& tileKey, HWND hwndSubscriber);
	HBITMAP getBlocking(const TileKey& tileKey);
	void unqueueInvisible(const TileRange& visibleTiles, HWND hwndSubscriber);
	void clear();
	void cleanUpCacheIfRequired();
	void onDownloadFinished();

  private:
	DownloadWorker& m_downloadWorker;
	const TileFetcher& m_tileFetcher;

	std::map<TileKey, CacheContent> m_cache;
	int addedEntriesSinceLastCleanup;
};
