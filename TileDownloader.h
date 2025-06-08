#pragma once

#include <windows.h>
#include <wininet.h>

#include "GdiPlusWrapper.h"
#include "TileKey.h"

class TileDownloader {
  public:
	TileDownloader(const GdiPlusWrapper* gdi);
	~TileDownloader();
	HBITMAP get(TileKey tileKey) const;

  private:
	const GdiPlusWrapper* m_gdi;
	HINTERNET m_hInternet;
};
