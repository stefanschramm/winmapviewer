#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include "Common.h"
#include "Settings.h"
#include "TileCache.h"

class MapControl {
  public:
	MapControl(HINSTANCE hInstance, HWND hwndMain, TileCache& tileCache);
	~MapControl();

	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HWND create(int x, int y, int width, int height);
	void requestRedraw();
	void setOffset(int offsetX, int offsetY);
	void moveToOffset();
	void setCenterLonLat(const LonLat* lonLat);
	void zoomIn();
	void zoomOut();
	void getSettings(Settings* settings) const;
	void setSettings(Settings* settings);
	void setStyle(const std::string& styleUrlTemplate);
	void setMaxZoomLevel(int maxZoomLevel);

	HINSTANCE m_hInstance;
	HWND m_hwndMap;
	HWND m_hwndMain;

  private:
	TileCache& m_tileCache;

	void render(HDC hdcDestination, RECT* updateRect);
	void setViewportSize(int width, int height);
	void getLonLat(int x, int y, LonLat* lonLat) const;
	void startDragging(int x, int y);
	bool mouseMove(int x, int y);
	void endDragging(int x, int y);
	void restrictCoordinates(long* x, long* y) const;
	void invalidateUpdateRects(const TileKey& tileKey) const;

	std::string m_styleUrlTemplate;
	int m_maxZoomLevel;

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
	LonLat m_clickLonLat;
};
