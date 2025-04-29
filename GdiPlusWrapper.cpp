#include "GdiPlusWrapper.h"

GdiPlusWrapper::GdiPlusWrapper() {
	m_lib = LoadLibrary(TEXT("gdiplus.dll"));
	if (m_lib == NULL) {
		MessageBox(NULL, TEXT("Unable to load library"), TEXT("winmapviewer"), MB_OK);
		throw "Unable to load gdiplus.dll";
	}

	// Get function pointers
	m_GdiplusStartup = (GdiplusStartupFunc)GetProcAddress(m_lib, "GdiplusStartup");
	m_GdipCreateBitmapFromStream = (GdipCreateBitmapFromStreamFunc)GetProcAddress(m_lib, "GdipCreateBitmapFromStream");
	m_GdipCreateHBITMAPFromBitmap = (GdipCreateHBITMAPFromBitmapFunc)GetProcAddress(m_lib, "GdipCreateHBITMAPFromBitmap");
	m_GdipDisposeImage = (GdipDisposeImageFunc)GetProcAddress(m_lib, "GdipDisposeImage");
	m_GdiplusShutdown = (GdiplusShutdownFunc)GetProcAddress(m_lib, "GdiplusShutdown");

	if (!m_GdiplusStartup || !m_GdipCreateBitmapFromStream || !m_GdipCreateHBITMAPFromBitmap || !m_GdipDisposeImage || !m_GdiplusShutdown) {
		MessageBox(NULL, TEXT("Unable to get GDI+ function pointers"), TEXT("winmapviewer"), MB_OK);
		throw "Unable to get GDI+ function pointers";
	}

	GdiplusStartupInput startupInput = {1, NULL, FALSE, FALSE};
	if (m_GdiplusStartup(&m_gdipToken, &startupInput, NULL) != 0) {
		MessageBox(NULL, TEXT("Unable to initialize GDI+"), TEXT("winmapviewer"), MB_OK);
		throw "Unable to initialize GDI+";
	}
}

GdiPlusWrapper::~GdiPlusWrapper() {
	m_GdiplusShutdown(m_gdipToken);
	FreeLibrary(m_lib);
}

HBITMAP GdiPlusWrapper::loadPng(IStream* stream) {
	GdiplusImage* image = NULL;
	HBITMAP hBitmap = NULL;

	if (m_GdipCreateBitmapFromStream(stream, &image) != 0 || image == NULL) {
		MessageBox(NULL, TEXT("Unable to load image"), TEXT("winmapviewer"), MB_OK);
		throw "Unable to load image";
	}

	if (m_GdipCreateHBITMAPFromBitmap(image, &hBitmap, 0) != 0 || hBitmap == NULL) {
		MessageBox(NULL, TEXT("Unable convert to HBITMAP"), TEXT("winmapviewer"), MB_OK);
		throw "Unable convert to HBITMAP";
	}

	m_GdipDisposeImage(image);

	return hBitmap;
}
