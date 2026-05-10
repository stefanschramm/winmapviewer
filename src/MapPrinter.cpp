// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <iostream>

#include "Common.h"
#include "MapPrinter.h"
#include "TileIterator.h"

MapPrinter::MapPrinter(TileCache& tileCache) : m_tileCache(tileCache) {
}

BOOL CALLBACK printAbortProc(HDC hdc, int code) {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void MapPrinter::print(
	HWND hwndParent,
	int zoomLevel,
	long centerX,
	long centerY,
	const std::string& styleUrlTemplate
) const {
	PRINTDLG pd;
	memset(&pd, 0, sizeof(pd));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwndParent;
	pd.Flags = PD_RETURNDC | PD_PRINTSETUP;
	pd.nCopies = 1;

	if (!PrintDlg(&pd)) {
		return;
	}

	HDC hdcPrint = pd.hDC;
	if (hdcPrint == NULL) {
		return;
	}

	if (SetAbortProc(hdcPrint, printAbortProc) == SP_ERROR) {
		warningMessage("Unable to set printing abort procedure.");
		DeleteDC(hdcPrint);
	}

	static DOCINFO di = {sizeof(DOCINFO), TEXT("WinMapViewer")};

	if (StartDoc(hdcPrint, &di) <= 0) {
		warningMessage("Unable to start document for print job.");
		DeleteDC(hdcPrint);
		return;
	}

	if (StartPage(hdcPrint) <= 0) {
		warningMessage("Unable to start page for print job.");
		DeleteDC(hdcPrint);
		return;
	}

	if (!renderPage(hdcPrint, zoomLevel, centerX, centerY, styleUrlTemplate)) {
		AbortDoc(hdcPrint);
		DeleteDC(hdcPrint);
		return;
	}

	if (EndPage(hdcPrint) <= 0) {
		warningMessage("Unable to end page of print job.");
		DeleteDC(hdcPrint);
		return;
	}

	if (EndDoc(hdcPrint) <= 0) {
		warningMessage("Unable to end document of print job.");
	}

	DeleteDC(hdcPrint);
}

bool MapPrinter::renderPage(HDC hdcPrint, int zoomLevel, long centerX, long centerY, const std::string& styleUrlTemplate) const {
	HDC hMemDC = CreateCompatibleDC(hdcPrint);

	int width = GetDeviceCaps(hdcPrint, HORZRES);
	int height = GetDeviceCaps(hdcPrint, VERTRES);

	TileIterator tileIterator(
		zoomLevel,
		centerX - width / 2,
		centerY - height / 2,
		GetDeviceCaps(hdcPrint, HORZRES),
		height
	);

	int tileX;
	int tileY;
	RECT tileRect;
	while (tileIterator.next(&tileX, &tileY, &tileRect)) {
		if (!printAbortProc(hdcPrint, 0)) {
			return false;
		}
		TileKey tileKey(
			styleUrlTemplate,
			zoomLevel,
			tileX,
			tileY
		);

		HBITMAP hBitmap = m_tileCache.getBlocking(tileKey);

		SelectObject(hMemDC, hBitmap);
		BitBlt(
			hdcPrint,
			tileRect.left,
			tileRect.top,
			tileRect.right - tileRect.left,
			tileRect.bottom - tileRect.top,
			hMemDC,
			0,
			0,
			SRCCOPY
		);
	}

	DeleteDC(hMemDC);

	return true;
}
