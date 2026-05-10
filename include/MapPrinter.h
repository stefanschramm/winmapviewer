#pragma once

#include "Settings.h"
#include "TileCache.h"

class MapPrinter {
  public:
	MapPrinter(TileCache& tileCache);
	void print(
		HWND hwndParent,
		int zoomLevel,
		long centerX,
		long centerY,
		const std::string& styleUrlTemplate
	) const;

  private:
	TileCache& m_tileCache;

	bool renderPage(HDC hdcPrint, int zoomLevel, long centerX, long centerY, const std::string& styleUrlTemplate) const;
};
