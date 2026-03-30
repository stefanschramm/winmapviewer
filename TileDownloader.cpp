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
}

TileDownloader::~TileDownloader() {
	InternetCloseHandle(m_hInternet);
}

std::string parseStyleUrlTemplate(std::string styleUrlTemplate, TileKey tileKey) {
	static const char* invalidPlaceholder = "Invalid URL template: Encountered invalid placeholder. Valid placeholders: {z}, {x}, {y}";

	std::stringstream strstr;
	size_t from = 0;
	for (int i = 0; i < 3; i++) {
		size_t placeholderStart = styleUrlTemplate.find("{", from);
		if (placeholderStart == std::string::npos) {
			throw "Invalid URL template: Expected to find (another) placeholder. Placeholders {z}, {x} and {y} should be set.";
		}
		size_t placeholderEnd = styleUrlTemplate.find("}", placeholderStart);
		if (placeholderEnd == std::string::npos) {
			throw "Invalid URL template: Closing bracket of placeholder not found.";
		}
		if (placeholderEnd - placeholderStart != 2) {
			throw invalidPlaceholder;
		}
		char c = styleUrlTemplate[placeholderStart + 1];
		int value;
		switch (c) {
			case 'z':
				value = tileKey.zoomLevel;
				break;
			case 'x':
				value = tileKey.x;
				break;
			case 'y':
				value = tileKey.y;
				break;
			default:
				throw invalidPlaceholder;
				break;
		}
		strstr << styleUrlTemplate.substr(from, placeholderStart - from) << value;
		from = placeholderEnd + 1;
	}
	strstr << styleUrlTemplate.substr(from);

	return strstr.str();
}

// Returns bitmap for specified tile
// The caller is responsible to DeleteObject after usage.
HBITMAP TileDownloader::get(TileKey tileKey) const {
	std::string url = parseStyleUrlTemplate(tileKey.styleUrlTemplate, tileKey);

	HINTERNET hUrl = InternetOpenUrl(m_hInternet, url.c_str(), NULL, 0, 0, 0);
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
