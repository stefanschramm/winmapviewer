#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <string>
#include <windows.h>
#include <wininet.h>

#include "TileKey.h"

class TileDownloader {
  public:
	TileDownloader();
	~TileDownloader();
	HBITMAP get(const TileKey& tileKey) const;

  private:
	HINTERNET m_hInternet;
};
