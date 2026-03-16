#include <objbase.h>
#include <sstream>
#include <string>

#include "Common.h"
#include "GdiPlusWrapper.h"
#include "TileDownloader.h"

TileDownloader::TileDownloader(const GdiPlusWrapper* gdi) : m_gdi(gdi) {
	m_hInternet = InternetOpen(TEXT("winmapviewer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hInternet) {
		throw "Unable to initialize WinINet.";
	}

	setStyle("https://tile.openstreetmap.org/{z}/{x}/{y}.png");
}

TileDownloader::~TileDownloader() {
	InternetCloseHandle(m_hInternet);
}

// Returns bitmap for specified tile
// The caller is responsible to DeleteObject after usage.
HBITMAP TileDownloader::get(TileKey tileKey) const {
	int tmp[3] = {tileKey.zoomLevel, tileKey.x, tileKey.y};

	// TODO: prevent buffer overflow
	char url[512];
	wsprintf(
		url,
		TEXT(m_urlFormatString.c_str()),
		tmp[m_urlFormatMap[0]],
		tmp[m_urlFormatMap[1]],
		tmp[m_urlFormatMap[2]]
	);

	HINTERNET hUrl = InternetOpenUrl(m_hInternet, url, NULL, 0, 0, 0);
	if (!hUrl) {
		return createPlaceholderBitmap(true);
	}

	IStream* memoryStream = NULL;
	if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &memoryStream))) {
		InternetCloseHandle(hUrl);
		throw "Failed to create memory stream.";
	}

	char buffer[10240];
	DWORD bytesRead = 0;
	ULONG bytesWritten = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		if (memoryStream->Write(buffer, bytesRead, &bytesWritten) != S_OK) {
			memoryStream->Release();
			InternetCloseHandle(hUrl);
			throw "Failed to write to memory stream.";
		}
	}

	LARGE_INTEGER liZero = {0, 0};
	memoryStream->Seek(liZero, STREAM_SEEK_SET, NULL);

	HBITMAP hBitmap = m_gdi->loadPng(memoryStream);
	if (hBitmap == NULL) {
		return createPlaceholderBitmap(true);
	}

	InternetCloseHandle(hUrl);

	return hBitmap;
}

// Set URL template for tiles in the form like
// https://tile.openstreetmap.org/{z}/{x}/{y}.png
//
// A sprintf format string will be generated once to prevent parsing the URL template on every get() call
void TileDownloader::setStyle(const std::string& urlTemplate) {
	static const char* invalidPlaceholder = "Invalid URL template: Encountered invalid placeholder. Valid placeholders: {z}, {x}, {y}";

	std::stringstream strstr;
	size_t from = 0;
	for (int i = 0; i < 3; i++) {
		size_t placeholderStart = urlTemplate.find("{", from);
		if (placeholderStart == std::string::npos) {
			throw "Invalid URL template: Expected to find (another) placeholder. Placeholders {z}, {x} and {y} should be set.";
		}
		size_t placeholderEnd = urlTemplate.find("}", placeholderStart);
		if (placeholderEnd == std::string::npos) {
			throw "Invalid URL template: Closing bracket of placeholder not found.";
		}
		if (placeholderEnd - placeholderStart != 2) {
			throw invalidPlaceholder;
		}
		char c = urlTemplate[placeholderStart + 1];
		switch (c) {
			case 'z':
				m_urlFormatMap[0] = i;
				break;
			case 'x':
				m_urlFormatMap[1] = i;
				break;
			case 'y':
				m_urlFormatMap[2] = i;
				break;
			default:
				throw invalidPlaceholder;
				break;
		}
		strstr << urlTemplate.substr(from, placeholderStart - from) << "%i";
		from = placeholderEnd + 1;
	}
	strstr << urlTemplate.substr(from);
	m_urlFormatString = strstr.str();
}
