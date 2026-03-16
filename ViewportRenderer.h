#pragma once

#include "Common.h"
#include "TileCache.h"

class ViewportRenderer {
  public:
	ViewportRenderer(int zoomLevel, HWND hWnd);
	~ViewportRenderer();
	void render(HWND hWnd);
	void render(HDC hdcDestination, RECT* updateRect);
	void setOffset(int offsetX, int offsetY);
	void moveToOffset();
	void setCenterLonLat(const LonLat* lonLat);
	void zoomIn();
	void zoomOut();
	void setViewportSize(int width, int height);
	void getLonLat(int x, int y, LonLat* lonLat) const;
	void startDragging(int x, int y);
	bool mouseMove(int x, int y);
	void endDragging(int x, int y);
	void setStyle(const std::string& urlTemplate);

  private:
	GdiPlusWrapper* m_gdi;
	TileDownloader* m_tileDownloader;
	DownloadWorker* m_downloadWorker;
	TileCache* m_tileCache;

	void restrictCoordinates(long* x, long* y) const;
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
