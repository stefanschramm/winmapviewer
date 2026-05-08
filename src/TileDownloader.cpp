#include <objbase.h>
#include <sstream>
#include <string>

#include "Common.h"
#include "TileDownloader.h"

#include "lib/image/stb_image.h"

TileDownloader::TileDownloader() {
	m_hInternet = InternetOpen(TEXT("winmapviewer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hInternet) {
		throw "Unable to initialize WinINet.";
	}
}

TileDownloader::~TileDownloader() {
	InternetCloseHandle(m_hInternet);
}

// Returns bitmap for specified tile
// The caller is responsible to DeleteObject after usage.
HBITMAP TileDownloader::get(const TileKey& tileKey) const {
	std::string url = parseStyleUrlTemplate(tileKey);

	HINTERNET hUrl = InternetOpenUrl(m_hInternet, url.c_str(), NULL, 0, 0, 0);
	if (!hUrl) {
		return createPlaceholderBitmap(true);
	}

	std::string rawData;
	char buffer[10240];
	DWORD bytesRead = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		rawData.append(buffer, bytesRead);
	}
	InternetCloseHandle(hUrl);

	int width;
	int height;
	int actualChannels;
	const int desiredChannels = STBI_rgb;
	stbi_uc* img = stbi_load_from_memory((stbi_uc*)rawData.c_str(), rawData.size(), &width, &height, &actualChannels, desiredChannels);
	if (img == NULL) {
		return createPlaceholderBitmap(true);
	}

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 8 * desiredChannels;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* dibPixels = NULL;
	HBITMAP hBmp = CreateDIBSection(
		NULL,
		&bmi,
		DIB_RGB_COLORS,
		&dibPixels,
		NULL,
		0
	);
	if (!hBmp) {
		return createPlaceholderBitmap(true);
	}

	memcpy(dibPixels, img, width * height * desiredChannels);

	stbi_image_free(img);

	// Swap red and blue channel because that's the format Windows expects
	unsigned char* p = (unsigned char*)dibPixels;
	int total = width * height;
	for (int i = 0; i < total; i++) {
		std::swap(p[0], p[2]);
		p += desiredChannels;
	}

	return hBmp;
}
