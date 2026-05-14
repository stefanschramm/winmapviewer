#pragma once

#include <windows.h>
#include <wininet.h>

#include "TileKey.h"

class TileFetcher {
  public:
	TileFetcher();
	~TileFetcher();
	HBITMAP get(const TileKey& tileKey) const;

  private:
	HBITMAP download(const std::string& url) const;
	HBITMAP loadFromFilesystem(const std::string& url) const;
	HINTERNET m_hInternet;
};
