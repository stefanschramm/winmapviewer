#include <math.h>

#include "ViewportRenderer.h"

// used for optimized multiplication
const unsigned int TILE_SIZE_BITS = 8;

// == 256 px
const unsigned int TILE_SIZE = 1 << TILE_SIZE_BITS;
const unsigned int TILE_INNER_OFFSET_MAP = 0xff;

// M_PI and asinh are missing in math.h of VC++ 6
const double M_PI = 3.141592653589793;

double asinh(double x) {
	return log(x + sqrt(x * x + 1));
}

ViewportRenderer::ViewportRenderer(TileCache& tileCache, int zoomLevel)
	: m_tileCache(tileCache),
	  m_viewportWidth(640),
	  m_viewportHeight(480),
	  m_offsetX(0),
	  m_offsetY(0),
	  m_zoomLevel(zoomLevel) {
	m_x = 0;
	m_y = 0;
}

void ViewportRenderer::render(HDC hdcDestination) {
	// top left corner of complete map
	long originX = m_x + m_offsetX;
	long originY = m_y + m_offsetY;

	restrictCoordinates(&originX, &originY);

	// top left tile
	int originTileX = originX >> TILE_SIZE_BITS;
	int originTileY = originY >> TILE_SIZE_BITS;

	// offset within the top left tile
	long offsetX = originX & TILE_INNER_OFFSET_MAP;
	long offsetY = originY & TILE_INNER_OFFSET_MAP;

	HDC hMemDC = CreateCompatibleDC(hdcDestination);

	// render one additional row/column of tiles at each edge
	for (int x = 0; x < (m_viewportWidth >> TILE_SIZE_BITS) + 2; x++) {
		for (int y = 0; y < (m_viewportHeight >> TILE_SIZE_BITS) + 2; y++) {
			int tileX = (originTileX + x) % (1 << m_zoomLevel);
			int tileY = originTileY + y;
			if (tileY > (1 << m_zoomLevel) - 1) {
				// south out of bounds
				RECT rect = {
				  -offsetX + (x << TILE_SIZE_BITS),
				  -offsetY + (y << TILE_SIZE_BITS),
				  -offsetX + (x << TILE_SIZE_BITS) + TILE_SIZE,
				  -offsetY + (y << TILE_SIZE_BITS) + TILE_SIZE};
				HBRUSH hBrush = CreateSolidBrush(RGB(128, 128, 128));
				FillRect(hdcDestination, &rect, hBrush);
				DeleteObject(hBrush);

				continue;
			}

			TileKey tileKey(m_zoomLevel, tileX, tileY);

			HBITMAP hBitmap = m_tileCache.get(tileKey);
			SelectObject(hMemDC, hBitmap);
			BitBlt(
			  hdcDestination,
			  -offsetX + (x << TILE_SIZE_BITS),
			  -offsetY + (y << TILE_SIZE_BITS),
			  TILE_SIZE,
			  TILE_SIZE,
			  hMemDC,
			  0,
			  0,
			  SRCCOPY
			);
		}
	}

	DeleteDC(hMemDC);
}

void ViewportRenderer::setOffset(int offsetX, int offsetY) {
	m_offsetX = offsetX;
	m_offsetY = offsetY;
}

void ViewportRenderer::moveToOffset() {
	m_x += m_offsetX;
	m_y += m_offsetY;
	restrictCoordinates(&m_x, &m_y);
	m_offsetX = 0;
	m_offsetY = 0;
}

void ViewportRenderer::setCenterLonLat(double lon, double lat) {
	m_x = (1 << m_zoomLevel << TILE_SIZE_BITS) * (lon + 180.0) / 360.0 - (m_viewportWidth >> 1);
	m_y = (1 << m_zoomLevel << TILE_SIZE_BITS) * (1.0 - asinh(tan(lat * M_PI / 180.0)) / M_PI) / 2.0 - (m_viewportHeight >> 1);
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::zoomIn() {
	if (m_zoomLevel >= 19) {
		return;
	}

	m_zoomLevel++;
	m_x = m_x << 1;
	m_y = m_y << 1;
	m_x = m_x + (m_viewportWidth >> 1);
	m_y = m_y + (m_viewportHeight >> 1);
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::zoomOut() {
	if (m_zoomLevel <= 0) {
		return;
	}

	m_zoomLevel--;
	m_x = m_x - (m_viewportWidth >> 1);
	m_y = m_y - (m_viewportHeight >> 1);
	m_x = m_x >> 1;
	m_y = m_y >> 1;
	restrictCoordinates(&m_x, &m_y);
}

void ViewportRenderer::setViewportSize(int width, int height) {
	// re-center
	m_x -= (width - m_viewportWidth) >> 1;
	m_y -= (height - m_viewportHeight) >> 1;
	restrictCoordinates(&m_x, &m_y);

	m_viewportWidth = width;
	m_viewportHeight = height;
}

void ViewportRenderer::restrictCoordinates(long* x, long* y) {
	long mapSize = 1 << m_zoomLevel << TILE_SIZE_BITS;

	// normalize x position
	if (*x < 0) {
		*x = (1 + mapSize) + *x;
	}
	*x = *x % mapSize;

	// restrict y position
	// Note: Depending on the viewportHeight and zoomLevel there still will be undrawn areas at the bottom.
	if (*y + m_viewportHeight > mapSize) {
		*y = mapSize - m_viewportHeight;
	}
	if (*y < 0) {
		*y = 0;
	}
}
