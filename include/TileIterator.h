#pragma once

#include <string>
#include <windows.h>

#include "TileRange.h"

class TileIterator {
  public:
	TileIterator(int zoomLevel, int x, int y, int width, int height);
	TileRange getTileRange(const std::string& styleUrlTemplate) const;
	int getOutOfRangeYOffset() const;
	bool next(int* tileX, int* tileY, RECT* destinationRect);

  private:
	int m_zoomLevel;

	// top left corner of complete map
	const long m_originX;
	const long m_originY;

	// top left tile
	const int m_originTileX;
	const int m_originTileY;

	// offset within the top left tile
	const int m_offsetX;
	const int m_offsetY;

	const int m_maxExtend;
	const int m_widthInTiles;
	const int m_heightInTiles;
	int m_outOfRangeYOffset;

	// iteration state
	int m_currentX;
	int m_currentY;
	bool m_iterationFinished;
};
