#pragma once

#include <objbase.h>

typedef DWORD GdiplusToken;

typedef struct GdiplusStartupInput {
	UINT GdiplusVersion;
	const void* DebugEventCallback;
	BOOL SuppressBackgroundThread;
	BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef struct GdiplusImage GdiplusImage;

typedef struct GdiplusStartupOutput {
	void* something;
} GdiplusStartupOutput;

typedef int(__stdcall* GdiplusStartupFunc)(GdiplusToken*, const GdiplusStartupInput*, GdiplusStartupOutput*);
typedef int(__stdcall* GdipCreateBitmapFromStreamFunc)(IStream*, GdiplusImage**);
typedef int(__stdcall* GdipCreateHBITMAPFromBitmapFunc)(GdiplusImage*, HBITMAP*, COLORREF);
typedef int(__stdcall* GdipDisposeImageFunc)(GdiplusImage*);
typedef void(__stdcall* GdiplusShutdownFunc)(GdiplusToken);

// Wrapper for dynamic loading of gdiplus.dll for older versions of Windows
class GdiPlusWrapper {
  public:
	GdiPlusWrapper();
	~GdiPlusWrapper();
	HBITMAP loadPng(IStream* stream);

  private:
	HINSTANCE m_lib;
	GdiplusStartupFunc m_GdiplusStartup;
	GdipCreateBitmapFromStreamFunc m_GdipCreateBitmapFromStream;
	GdipCreateHBITMAPFromBitmapFunc m_GdipCreateHBITMAPFromBitmap;
	GdipDisposeImageFunc m_GdipDisposeImage;
	GdiplusShutdownFunc m_GdiplusShutdown;
	GdiplusToken m_gdipToken;
};
