#include <fstream>
#include <sstream>
#include <windows.h>

#include "lib/image/stb_image.h"

#include "Common.h"
#include "TileFetcher.h"

TileFetcher::TileFetcher() {
	m_hInternet = InternetOpen(TEXT("winmapviewer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hInternet) {
		throw "Unable to initialize WinINet.";
	}
}

TileFetcher::~TileFetcher() {
	InternetCloseHandle(m_hInternet);
}

HBITMAP createPlaceholderBitmap() {
	int width = 256;
	int height = 256;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 8 * 3;
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

	unsigned char* p = (unsigned char*)dibPixels;
	int total = width * height;
	for (int i = 0; i < total; i++) {
		p[0] = 0xcc; // B
		p[1] = 0xcc; // G
		p[2] = 0xff; // R
		p += 3;
	}

	return hBmp;
}

HBITMAP readImageData(const std::string& rawData) {
	int width;
	int height;
	int actualChannels;
	const int desiredChannels = STBI_rgb;
	stbi_uc* img = stbi_load_from_memory((stbi_uc*)rawData.c_str(), rawData.size(), &width, &height, &actualChannels, desiredChannels);
	if (img == NULL) {
		return createPlaceholderBitmap();
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
		stbi_image_free(img);
		return createPlaceholderBitmap();
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

HBITMAP TileFetcher::download(const std::string& url) const {
	HINTERNET hUrl = InternetOpenUrl(m_hInternet, url.c_str(), NULL, 0, 0, 0);
	if (!hUrl) {
		return createPlaceholderBitmap();
	}

	std::string rawData;
	char buffer[10240];
	DWORD bytesRead = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		rawData.append(buffer, bytesRead);
	}
	InternetCloseHandle(hUrl);

	return readImageData(rawData);
}

HBITMAP TileFetcher::loadFromFilesystem(const std::string& fileName) const {
	std::stringstream rawData;
	std::ifstream file;
	file.open(fileName.c_str(), std::ios::binary);
	if (!file) {
		return createPlaceholderBitmap();
	}

	rawData << file.rdbuf();
	file.close();

	return readImageData(rawData.str());
}

// Returns bitmap for specified tile
// The caller is responsible to DeleteObject after usage.
HBITMAP TileFetcher::get(const TileKey& tileKey) const {
	std::string url = parseStyleUrlTemplate(tileKey);
	if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
		return download(url);
	}
	if (url.substr(0, 7) == "file://") {
		// Just pass the rest. For simplicity we don't care about URL-encoding for now.
		return loadFromFilesystem(url.substr(7));
	}

	// unknown scheme
	return createPlaceholderBitmap();
}
