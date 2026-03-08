#include "GdiPlusWrapper.h"

GdiPlusWrapper::GdiPlusWrapper() {
	m_lib = LoadLibrary(TEXT("gdiplus.dll"));
	if (m_lib == NULL) {
		throw "Unable to load gdiplus.dll";
	}

	// Get function pointers
	m_GdiplusStartup = (GdiplusStartupFunc)GetProcAddress(m_lib, "GdiplusStartup");
	m_GdipCreateBitmapFromStream = (GdipCreateBitmapFromStreamFunc)GetProcAddress(m_lib, "GdipCreateBitmapFromStream");
	m_GdipCreateHBITMAPFromBitmap = (GdipCreateHBITMAPFromBitmapFunc)GetProcAddress(m_lib, "GdipCreateHBITMAPFromBitmap");
	m_GdipDisposeImage = (GdipDisposeImageFunc)GetProcAddress(m_lib, "GdipDisposeImage");
	m_GdiplusShutdown = (GdiplusShutdownFunc)GetProcAddress(m_lib, "GdiplusShutdown");

	if (!m_GdiplusStartup || !m_GdipCreateBitmapFromStream || !m_GdipCreateHBITMAPFromBitmap || !m_GdipDisposeImage || !m_GdiplusShutdown) {
		throw "Unable to get GDI+ function pointers";
	}

	GdiplusStartupInput startupInput = {1, NULL, FALSE, FALSE};
	if (m_GdiplusStartup(&m_gdipToken, &startupInput, NULL) != 0) {
		throw "Unable to initialize GDI+";
	}
}

GdiPlusWrapper::~GdiPlusWrapper() {
	m_GdiplusShutdown(m_gdipToken);
	FreeLibrary(m_lib);
}

HBITMAP GdiPlusWrapper::loadPng(IStream* stream) const {
	GdiplusImage* image = NULL;
	HBITMAP hBitmap = NULL;

	if (m_GdipCreateBitmapFromStream(stream, &image) != 0 || image == NULL) {
		throw "Unable to load image";
	}

	if (m_GdipCreateHBITMAPFromBitmap(image, &hBitmap, 0) != 0 || hBitmap == NULL) {
		throw "Unable convert to HBITMAP";
	}

	m_GdipDisposeImage(image);

	return hBitmap;
}
