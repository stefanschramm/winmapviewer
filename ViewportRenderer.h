#pragma once

#include "TileCache.h"

class ViewportRenderer {
  public:
	ViewportRenderer(int zoomLevel, HWND hWnd);
	~ViewportRenderer();
	void render(HWND hWnd);
	void render(HDC hdcDestination);
	void setOffset(int offsetX, int offsetY);
	void moveToOffset();
	void setCenterLonLat(double lon, double lat);
	void zoomIn();
	void zoomOut();
	void setViewportSize(int width, int height);
	void getLonLat(int x, int y, double* lon, double* lat);
	void startDragging(int x, int y);
	bool mouseMove(int x, int y);
	void endDragging(int x, int y);

  private:
	GdiPlusWrapper* m_gdi;
	const TileDownloader* m_tileDownloader;
	DownloadWorker* m_downloadWorker;
	TileCache* m_tileCache;

	void restrictCoordinates(long* x, long* y);
	int m_zoomLevel;
	long m_x;
	long m_y;
	long m_viewportWidth;
	long m_viewportHeight;
	int m_offsetX;
	int m_offsetY;
	bool m_dragging;
	int m_dragStartX;
	int m_dragStartY;
};
