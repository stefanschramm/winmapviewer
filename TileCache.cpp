#include <algorithm>

#include "Common.h"
#include "TileCache.h"

TileCache::TileCache(DownloadWorker& downloadWorker)
	: m_downloadWorker(downloadWorker) {
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

	return m_cache[tileKey].bitmap;
}

void TileCache::unqueueInvisible(const TileRange& visibleTiles, HWND hwndSubscriber) {
	for (std::map<TileKey, CacheContent>::iterator iterator = m_cache.begin(); iterator != m_cache.end();) {
		if (!visibleTiles.contains(iterator->first) && contains(iterator->second.subscribers, hwndSubscriber)) {
			iterator->second.subscribers.erase(std::remove(iterator->second.subscribers.begin(), iterator->second.subscribers.end(), hwndSubscriber), iterator->second.subscribers.end());
		}
		if (!iterator->second.available && iterator->second.subscribers.empty()) {
			// No one else was waiting for this tile
			m_downloadWorker.unqueue(iterator->first);
			// Remove entry with reference to placeholder image
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

	for (std::map<TileKey, HBITMAP>::iterator finishedIterator = finishedDownloads.begin(); finishedIterator != finishedDownloads.end(); ++finishedIterator) {
		std::map<TileKey, CacheContent>::iterator cacheIterator = m_cache.find(finishedIterator->first);
		if (cacheIterator != m_cache.end()) {
			cacheIterator->second.bitmap = finishedIterator->second;
			cacheIterator->second.available = true;
			notifySubscribers(cacheIterator->first, cacheIterator->second.subscribers);
			cacheIterator->second.subscribers.clear();
		} else {
			// TODO: Currently this should not happen, because on every download start a cache entry is created.
			// When cache invalidation gets implemented we probably need to take care of this.
			// Directly Delete HBITMAP because it's not used anymore? Or just put it into cache?
		}
	}
}
