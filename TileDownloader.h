#pragma once

#include <string>
#include <windows.h>
#include <wininet.h>

#include "GdiPlusWrapper.h"
#include "TileKey.h"

class TileDownloader {
  public:
	TileDownloader(const GdiPlusWrapper* gdi);
	~TileDownloader();
	void setStyle(std::string urlTemplate);
	HBITMAP get(TileKey tileKey) const;

  private:
	int m_urlFormatMap[3];
	std::string m_urlFormatString;
	const GdiPlusWrapper* m_gdi;
	HINTERNET m_hInternet;
};
