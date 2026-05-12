#include "TileIterator.h"
#include "Common.h"

// used for optimized multiplication
const int TILE_SIZE_BITS = 8;

// == 256 px
const int TILE_SIZE = 1 << TILE_SIZE_BITS;
const int TILE_INNER_OFFSET_MAP = 0xff;

// VC++ 6 compatibility
int myMin(int a, int b) {
	return a < b ? a : b;
}

TileIterator::TileIterator(int zoomLevel, int x, int y, int width, int height)
	: m_zoomLevel(zoomLevel),
	  m_originX(x),
	  m_originY(y),
	  m_originTileX(m_originX >> TILE_SIZE_BITS),
	  m_originTileY(m_originY >> TILE_SIZE_BITS),
	  m_offsetX(m_originX & TILE_INNER_OFFSET_MAP),
	  m_offsetY(m_originY & TILE_INNER_OFFSET_MAP),
	  m_maxExtend(1 << m_zoomLevel),
	  m_widthInTiles((width >> TILE_SIZE_BITS) + 2),
	  m_heightInTiles((height >> TILE_SIZE_BITS) + 2),
	  m_currentX(0),
	  m_currentY(0),
	  m_iterationFinished(false) {
	long fullMapHeight = m_maxExtend << TILE_SIZE_BITS;
	m_outOfRangeYOffset = fullMapHeight < height ? fullMapHeight : 0;
}

TileRange TileIterator::getTileRange(const std::string& styleUrlTemplate) const {
	TileRange visibleTiles(
		styleUrlTemplate,
		m_zoomLevel,
		m_originTileX,
		m_originTileX + m_widthInTiles,
		m_originTileY,
		myMin(m_originTileY + m_heightInTiles, m_maxExtend)
	);

	return visibleTiles;
}

int TileIterator::getOutOfRangeYOffset() const {
	return m_outOfRangeYOffset;
}

bool TileIterator::next(int* tileX, int* tileY, RECT* destinationRect) {
	if (m_iterationFinished) {
		return false;
	}

	*tileX = (m_originTileX + m_currentX) % m_maxExtend;
	*tileY = m_originTileY + m_currentY;

	destinationRect->left = -m_offsetX + (m_currentX << TILE_SIZE_BITS);
	destinationRect->top = -m_offsetY + (m_currentY << TILE_SIZE_BITS);
	destinationRect->right = -m_offsetX + (m_currentX << TILE_SIZE_BITS) + TILE_SIZE;
	destinationRect->bottom = -m_offsetY + (m_currentY << TILE_SIZE_BITS) + TILE_SIZE;

	m_currentX++;
	if (m_currentX > m_widthInTiles) {
		m_currentX = 0;
		m_currentY++;
		if (m_currentY > m_heightInTiles) {
			m_iterationFinished = true;
			return false;
		}
	}
	if (*tileY > m_maxExtend - 1) {
		m_iterationFinished = true;
		return false;
	}

	return true;
}
