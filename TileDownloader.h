#pragma once

#include <windows.h>
#include <wininet.h>

#include "GdiPlusWrapper.h"
#include "TileKey.h"

class TileDownloader {
  public:
	TileDownloader(GdiPlusWrapper* gdi);
	~TileDownloader();
	HBITMAP get(TileKey tileKey);

  private:
	GdiPlusWrapper* m_gdi;
	HINTERNET m_hInternet;
};
