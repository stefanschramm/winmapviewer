#pragma once

#include "TileCache.h"

class ViewportRenderer {
  public:
	ViewportRenderer(TileCache& tileCache, int zoomLevel);
	void render(HWND hWnd);
	void render(HDC hdcDestination);
	void setOffset(int offsetX, int offsetY);
	void moveToOffset();
	void setCenterLonLat(double lon, double lat);
	void zoomIn();
	void zoomOut();
	void setViewportSize(int width, int height);
	void getLonLat(int x, int y, double* lon, double* lat);

  private:
	void restrictCoordinates(long* x, long* y);
	TileCache& m_tileCache;
	int m_zoomLevel;
	long m_x;
	long m_y;
	long m_viewportWidth;
	long m_viewportHeight;
	int m_offsetX;
	int m_offsetY;
};
