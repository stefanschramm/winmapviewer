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

std::string parseStyleUrlTemplate(const TileKey& tileKey) {
	static const char* invalidPlaceholder = "Encountered invalid placeholder. Valid required placeholders: {z}, {x}, {y}; Optional Placeholder: {s}";

	int requiredPlaceholdersFound = 0;
	std::stringstream strstr;
	size_t from = 0;
	while (true) {
		size_t placeholderStart = tileKey.styleUrlTemplate.find("{", from);
		if (placeholderStart == std::string::npos) {
			break;
		}
		size_t placeholderEnd = tileKey.styleUrlTemplate.find("}", placeholderStart);
		if (placeholderEnd == std::string::npos) {
			throw "Closing bracket of placeholder not found.";
		}
		if (placeholderEnd - placeholderStart != 2) {
			throw invalidPlaceholder;
		}
		strstr << tileKey.styleUrlTemplate.substr(from, placeholderStart - from);
		char c = tileKey.styleUrlTemplate[placeholderStart + 1];
		switch (c) {
			case 'z':
				strstr << tileKey.zoomLevel;
				requiredPlaceholdersFound++;
				break;
			case 'x':
				strstr << tileKey.x;
				requiredPlaceholdersFound++;
				break;
			case 'y':
				strstr << tileKey.y;
				requiredPlaceholdersFound++;
				break;
			case 's':
				// Optional placeholder for server subdomain
				// Simply use the same server because we're not doing concurrent requests.
				strstr << "a";
				break;
			default:
				throw invalidPlaceholder;
				break;
		}
		from = placeholderEnd + 1;
	}

	if (requiredPlaceholdersFound != 3) {
		throw "Expected to find (another) placeholder. Placeholders {z}, {x} and {y} should be set.";
	}

	strstr << tileKey.styleUrlTemplate.substr(from);

	return strstr.str();
}

void panicMessage(const char* place, const char* message) {
	std::cerr << "Exception caught in " << place << ": " << message << std::endl;
	MessageBoxA(NULL, message, "winmapviewer", MB_OK);
}
