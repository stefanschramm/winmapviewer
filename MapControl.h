#define WM_MAP_ZOOM_IN (WM_USER + 10)
#define WM_MAP_ZOOM_OUT (WM_USER + 11)
#define WM_MAP_MOVE_X (WM_USER + 12)
#define WM_MAP_MOVE_Y (WM_USER + 13)
#define WM_MAP_LONLAT_UPDATE (WM_USER + 14)
#define WM_MAP_GET_LONLAT (WM_USER + 15)
#define WM_MAP_PRINT (WM_USER + 16)

struct LonLat {
	double lon;
	double lat;
};

void RegisterMapControl(HINSTANCE hInstance);
HWND CreateMapWindow(int x, int y, int width, int height, HWND hWnd, HINSTANCE hInstance);
