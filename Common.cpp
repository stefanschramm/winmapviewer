#include "Common.h"

HBITMAP createPlaceholderBitmap(bool error) {
	HBITMAP hPlaceholderBitmap = CreateBitmap(256, 256, 1, 32, NULL);
	HDC hdc = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hPlaceholderBitmap);
	HBRUSH hBrush = CreateSolidBrush(RGB(error ? 0xff : 0xcc, 0xcc, 0xcc));
	RECT rect = {0, 0, 256, 256};
	FillRect(hMemDC, &rect, hBrush);
	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBrush);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hdc);

	return hPlaceholderBitmap;
}

std::wstring urlEncode(const std::wstring& url) {
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

	int wLength = MultiByteToWideChar(CP_UTF8, 0, encoded.c_str(), -1, 0, 0);
	std::wstring result(wLength - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, encoded.c_str(), -1, &result[0], wLength);

	return result;
}
