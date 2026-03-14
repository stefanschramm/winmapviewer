#include "Utils.h"

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
