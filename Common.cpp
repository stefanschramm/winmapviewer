#include <iostream>
#include <sstream>

#include "Common.h"

HBITMAP createPlaceholderBitmap(bool error) {
	HBITMAP hPlaceholderBitmap = CreateBitmap(256, 256, 1, 32, NULL);
	HDC hdc = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hMemDC, hPlaceholderBitmap));
	HBRUSH hBrush = CreateSolidBrush(RGB(error ? 0xff : 0xcc, 0xcc, 0xcc));
	RECT rect = {0, 0, 256, 256};
	FillRect(hMemDC, &rect, hBrush);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBrush);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hdc);

	return hPlaceholderBitmap;
}

std::string urlEncode(const std::wstring& url) {
	int utf8Length = WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, 0, 0, 0, 0);
	std::string utf8(utf8Length - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, url.c_str(), -1, &utf8[0], utf8Length, 0, 0);

	static const char hex[] = "0123456789ABCDEF";

	std::string encoded;
	for (size_t i = 0; i < utf8.size(); ++i) {
		unsigned char c = utf8[i];

		if ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') ||
			c == '-' || c == '_' || c == '.' || c == '~') {
			encoded += c;
		} else {
			encoded += '%';
			encoded += hex[c >> 4];
			encoded += hex[c & 0x0F];
		}
	}

	return encoded;
}

std::string parseStyleUrlTemplate(const TileKey& tileKey) {
	static const char* invalidPlaceholder = "Encountered invalid placeholder. Valid placeholders: {z}, {x}, {y}";

	std::stringstream strstr;
	size_t from = 0;
	for (int i = 0; i < 3; i++) {
		size_t placeholderStart = tileKey.styleUrlTemplate.find("{", from);
		if (placeholderStart == std::string::npos) {
			throw "Expected to find (another) placeholder. Placeholders {z}, {x} and {y} should be set.";
		}
		size_t placeholderEnd = tileKey.styleUrlTemplate.find("}", placeholderStart);
		if (placeholderEnd == std::string::npos) {
			throw "Closing bracket of placeholder not found.";
		}
		if (placeholderEnd - placeholderStart != 2) {
			throw invalidPlaceholder;
		}
		char c = tileKey.styleUrlTemplate[placeholderStart + 1];
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
		strstr << tileKey.styleUrlTemplate.substr(from, placeholderStart - from) << value;
		from = placeholderEnd + 1;
	}
	strstr << tileKey.styleUrlTemplate.substr(from);

	return strstr.str();
}

void panicMessage(const char* place, const char* message) {
	std::cerr << "Exception caught in " << place << ": " << message << std::endl;
	MessageBoxA(NULL, message, "winmapviewer", MB_OK);
}
