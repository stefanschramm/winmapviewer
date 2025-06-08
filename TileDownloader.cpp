#include <objbase.h>

#include "GdiPlusWrapper.h"
#include "TileDownloader.h"

TileDownloader::TileDownloader(const GdiPlusWrapper* gdi) : m_gdi(gdi) {
	m_hInternet = InternetOpen(TEXT("winmapviewer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hInternet) {
		MessageBox(NULL, TEXT("Unable to initialize WinINet"), TEXT("winmapviewer"), MB_OK);
		throw "Unable to initialize WinINet";
	}
}

TileDownloader::~TileDownloader() {
	InternetCloseHandle(m_hInternet);
}

// Returns bitmap for specified tile
// The caller is responsible to DeleteObject after usage.
HBITMAP TileDownloader::get(TileKey tileKey) const {
	char url[128];
	wsprintf(
		url,
		TEXT("http://tile.openstreetmap.org/%i/%i/%i.png"),
		tileKey.zoomLevel,
		tileKey.x,
		tileKey.y
	);

	HINTERNET hUrl = InternetOpenUrl(m_hInternet, url, NULL, 0, 0, 0);
	if (!hUrl) {
		MessageBox(NULL, TEXT("Failed to open URL"), TEXT("winmapviewer"), MB_OK);
		throw "Failed to open URL";
	}

	IStream* memoryStream = NULL;
	if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &memoryStream))) {
		MessageBox(NULL, TEXT("Failed to create memory stream."), TEXT("winmapviewer"), MB_OK);
		InternetCloseHandle(hUrl);
		throw "Failed to create memory stream.";
	}

	char buffer[10240];
	DWORD bytesRead = 0;
	ULONG bytesWritten = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		if (memoryStream->Write(buffer, bytesRead, &bytesWritten) != S_OK) {
			MessageBox(NULL, TEXT("Failed to write to memory stream."), TEXT("winmapviewer"), MB_OK);
			memoryStream->Release();
			InternetCloseHandle(hUrl);
			throw "Failed to write to memory stream.";
		}
	}

	LARGE_INTEGER liZero = {0, 0};
	memoryStream->Seek(liZero, STREAM_SEEK_SET, NULL);

	HBITMAP hBitmap = m_gdi->loadPng(memoryStream);

	InternetCloseHandle(hUrl);

	return hBitmap;
}
