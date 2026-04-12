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
	// TODO: Remember last use and later clean up cache (keep 100 tiles)?
};

class TileCache {
  public:
	TileCache(DownloadWorker& downloadWorker);
	~TileCache();
	HBITMAP get(const TileKey& tileKey, HWND hwndSubscriber);
	void unqueueInvisible(const TileRange& visibleTiles, HWND hwndSubscriber);
	void clear();
	void onDownloadFinished();

  private:
	DownloadWorker& m_downloadWorker;

	std::map<TileKey, CacheContent> m_cache;
	HBITMAP m_hPlaceholderBitmap;
};
