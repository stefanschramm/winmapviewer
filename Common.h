#pragma once

#include <string>
#include <windows.h>

struct LonLat {
	double lon;
	double lat;
};

HBITMAP createPlaceholderBitmap(bool error);

std::string urlEncode(const std::wstring& url);
