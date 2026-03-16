#include "TileCache.h"
#include "Common.h"

TileCache::TileCache(const TileDownloader* tileDownloader, DownloadWorker* downloadWorker)
	: m_tileDownloader(tileDownloader),
	  m_downloadWorker(downloadWorker) {
	m_hPlaceholderBitmap = createPlaceholderBitmap(false);
}

TileCache::~TileCache() {
	clear();
	DeleteObject(m_hPlaceholderBitmap);
}

HBITMAP TileCache::get(TileKey tileKey) {
	if (tileKey.x < 0 || tileKey.y < 0 || tileKey.x > (1 << tileKey.zoomLevel) || tileKey.y > (1 << tileKey.zoomLevel)) {
		throw "Invalid tile requested";
	}

	m_downloadWorker->transferFinishedDownloads(&m_map);

	std::map<TileKey, HBITMAP>::iterator iterator = m_map.find(tileKey);
	if (iterator != m_map.end()) {
		// When the map contains either the real tile or the placeholder, no triggering of download is required.
		return iterator->second;
	}

	// download asynchronously
	m_downloadWorker->download(tileKey);

	m_map[tileKey] = m_hPlaceholderBitmap;

	return m_map[tileKey];
}

void TileCache::unqueueInvisible(TileRange visibleTiles) {
	m_downloadWorker->unqueueInvisible(visibleTiles, &m_map);
}

void TileCache::clear() {
	std::map<TileKey, HBITMAP>::iterator iterator;
	for (iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
		if (iterator->second != m_hPlaceholderBitmap) {
			// there may be multiple m_hPlaceholderBitmap in the map
			DeleteObject(iterator->second);
		}
	}
	m_map.clear();
}
