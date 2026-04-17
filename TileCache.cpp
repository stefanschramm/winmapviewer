#include <algorithm>

#include "Common.h"
#include "TileCache.h"

TileCache::TileCache(DownloadWorker& downloadWorker, const TileDownloader& tileDownloader)
	: m_downloadWorker(downloadWorker),
	  m_tileDownloader(tileDownloader) {
	m_hPlaceholderBitmap = createPlaceholderBitmap(false);
}

TileCache::~TileCache() {
	clear();
	DeleteObject(m_hPlaceholderBitmap);
}

bool contains(const Subscribers& subscribers, HWND hwndSubscriber) {
	return std::find(subscribers.begin(), subscribers.end(), hwndSubscriber) != subscribers.end();
}

HBITMAP TileCache::get(const TileKey& tileKey, HWND hwndSubscriber) {
	if (tileKey.x < 0 || tileKey.y < 0 || tileKey.x > (1 << tileKey.zoomLevel) || tileKey.y > (1 << tileKey.zoomLevel)) {
		throw "Invalid tile requested";
	}

	std::map<TileKey, CacheContent>::iterator iterator = m_cache.find(tileKey);
	if (iterator != m_cache.end()) {
		if (iterator->second.available) {
			return iterator->second.bitmap;
		}
		// Downloading, but not finished yet; tile possibly requested by another window
		if (!contains(iterator->second.subscribers, hwndSubscriber)) {
			iterator->second.subscribers.push_back(hwndSubscriber);
		}

		return m_hPlaceholderBitmap;
	}

	CacheContent cacheContent;
	cacheContent.available = false;
	cacheContent.bitmap = 0;
	cacheContent.subscribers.push_back(hwndSubscriber);
	m_cache[tileKey] = cacheContent;

	// download asynchronously
	m_downloadWorker.download(tileKey);

	// TODO: Check if a nearby zoom level has the corresponding tile(-quadruple)
	// and deliver a scaled version of it temporarily?
	// CacheContent should have a flag if it's the actual tile or a scaled version.

	return m_hPlaceholderBitmap;
}

HBITMAP TileCache::getBlocking(const TileKey& tileKey) {
	if (tileKey.x < 0 || tileKey.y < 0 || tileKey.x > (1 << tileKey.zoomLevel) || tileKey.y > (1 << tileKey.zoomLevel)) {
		throw "Invalid tile requested";
	}

	std::map<TileKey, CacheContent>::iterator iterator = m_cache.find(tileKey);
	if (iterator != m_cache.end() && iterator->second.available) {
		return iterator->second.bitmap;
	}

	HBITMAP hBitmap = m_tileDownloader.get(tileKey);

	CacheContent cacheContent;
	cacheContent.available = true;
	cacheContent.bitmap = hBitmap;
	m_cache[tileKey] = cacheContent;

	return hBitmap;
}

void TileCache::unqueueInvisible(const TileRange& visibleTiles, HWND hwndSubscriber) {
	for (std::map<TileKey, CacheContent>::iterator iterator = m_cache.begin(); iterator != m_cache.end();) {
		if (!visibleTiles.contains(iterator->first) && contains(iterator->second.subscribers, hwndSubscriber)) {
			iterator->second.subscribers.erase(std::remove(iterator->second.subscribers.begin(), iterator->second.subscribers.end(), hwndSubscriber), iterator->second.subscribers.end());
		}
		if (!iterator->second.available && iterator->second.subscribers.empty()) {
			// No one else was waiting for this tile
			m_downloadWorker.unqueue(iterator->first);
			m_cache.erase((iterator++)->first);
		} else {
			++iterator;
		}
	}
}

void TileCache::clear() {
	for (std::map<TileKey, CacheContent>::iterator iterator = m_cache.begin(); iterator != m_cache.end(); iterator++) {
		if (iterator->second.available) {
			DeleteObject(iterator->second.bitmap);
		}
	}
	m_cache.clear();
}

void notifySubscribers(const TileKey& tileKey, std::vector<HWND>& subscribers) {
	for (std::vector<HWND>::iterator it = subscribers.begin(); it != subscribers.end(); ++it) {
		SendMessage(*it, WM_USER_TILE_AVAILABLE, 0, reinterpret_cast<LPARAM>(&tileKey));
	}
}

void TileCache::onDownloadFinished() {
	std::map<TileKey, HBITMAP> finishedDownloads;
	m_downloadWorker.transferFinishedDownloads(&finishedDownloads);

	// Check all finished downloads and notify their subscribers
	for (std::map<TileKey, HBITMAP>::iterator finishedIterator = finishedDownloads.begin(); finishedIterator != finishedDownloads.end(); ++finishedIterator) {
		std::map<TileKey, CacheContent>::iterator cacheIterator = m_cache.find(finishedIterator->first);
		if (cacheIterator != m_cache.end()) {
			cacheIterator->second.bitmap = finishedIterator->second;
			cacheIterator->second.available = true;
			notifySubscribers(cacheIterator->first, cacheIterator->second.subscribers);
			cacheIterator->second.subscribers.clear();
		} else {
			// A download has finished that had no subscribers - keep tile for later usage
			CacheContent cacheContent;
			cacheContent.available = true;
			cacheContent.bitmap = finishedIterator->second;
			m_cache[finishedIterator->first] = cacheContent;
		}
	}
}
