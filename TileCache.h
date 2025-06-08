#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <map>

#include "DownloadWorker.h"
#include "TileDownloader.h"
#include "TileKey.h"

// TODO: Cache PNG data instead of bitmap to save memory.
// TODO: Clean up cache (keep 100 tiles?)
class TileCache {
  public:
	TileCache(const TileDownloader* tileDownloader, DownloadWorker* downloadWorker);
	~TileCache();
	HBITMAP get(TileKey tileKey);
	void downloadFinished(TileKey key, HBITMAP hBitmap);

  private:
	const TileDownloader* const m_tileDownloader;
	DownloadWorker* const m_downloadWorker;

	std::map<TileKey, HBITMAP> m_map;
	HBITMAP m_hPlaceholderBitmap;
};
