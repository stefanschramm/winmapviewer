#include "TileCache.h"

HBITMAP createPlaceholderBitmap() {
	HBITMAP m_hPlaceholderBitmap = CreateBitmap(256, 256, 1, 32, NULL);
	HDC hdc = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, m_hPlaceholderBitmap);
	HBRUSH hBrush = CreateSolidBrush(RGB(0xcc, 0xcc, 0xcc));
	RECT rect = {0, 0, 256, 256};
	FillRect(hMemDC, &rect, hBrush);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBrush);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hdc);

	return m_hPlaceholderBitmap;
}

TileCache::TileCache(TileDownloader& tileDownloader, DownloadWorker& downloadWorker)
	: m_tileDownloader(tileDownloader),
	  m_downloadWorker(downloadWorker) {
	m_hPlaceholderBitmap = createPlaceholderBitmap();
}

TileCache::~TileCache() {
	std::map<TileKey, HBITMAP>::iterator iterator;
	for (iterator = m_map.begin(); iterator != m_map.end(); iterator++) {
		if (iterator->second != m_hPlaceholderBitmap) {
			// there may be multiple m_hPlaceholderBitmap in the map
			DeleteObject(iterator->second);
		}
	}

	DeleteObject(m_hPlaceholderBitmap);
}

HBITMAP TileCache::get(TileKey tileKey) {
	if (tileKey.x < 0 || tileKey.y < 0 || tileKey.x > (1 << tileKey.zoomLevel) || tileKey.y > (1 << tileKey.zoomLevel)) {
		MessageBox(NULL, TEXT("Invalid tile requested"), TEXT("winmapviewer"), MB_OK);
		throw "Invalid tile requested";
	}

	m_downloadWorker.transferFinishedDownloads(&m_map);

	std::map<TileKey, HBITMAP>::iterator iterator = m_map.find(tileKey);
	if (iterator != m_map.end()) {
		// When the map contains either the real tile or the placeholder, no download is required.
		return iterator->second;
	}

	// download asynchronously
	m_downloadWorker.download(tileKey);

	m_map[tileKey] = m_hPlaceholderBitmap;

	return m_map[tileKey];
}
