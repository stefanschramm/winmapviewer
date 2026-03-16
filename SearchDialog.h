#pragma once

#include <wtypes.h>

// TODO: Put all messages in same include (s. MapControl.h)?
#define WM_SEARCH_SET_LONLAT (WM_USER + 30)

void search(HINSTANCE hInst, HWND hWnd);
