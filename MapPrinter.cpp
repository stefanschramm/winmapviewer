#include <iostream>

#include "MapPrinter.h"
#include "TileIterator.h"

MapPrinter::MapPrinter(TileCache& tileCache, const TileDownloader& tileDownloader) : m_tileCache(tileCache), m_tileDownloader(tileDownloader) {
}

void MapPrinter::print(
	HWND hwndParent,
	int zoomLevel,
	long centerX,
	long centerY,
	const std::string& styleUrlTemplate
) const {

	// TODO: Set abort procedure
	// TODO: Do it in a thread
	// TODO: Implement a timeout for downloading tiles?
	// TODO: Display modal dialog with cancel button

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
	static DOCINFO di = {sizeof(DOCINFO), TEXT("WinMapViewer")};
	if (hdcPrint == NULL) {
		return;
	}

	if (StartDoc(hdcPrint, &di) <= 0) {
		MessageBox(NULL, TEXT("StartDoc problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}

	if (StartPage(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("StartPage problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}

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
		TileKey tileKey(
			styleUrlTemplate,
			zoomLevel,
			tileX,
			tileY
		);

		HBITMAP hBitmap = m_tileCache.getFromCache(tileKey);
		if (hBitmap == NULL) {
			hBitmap = m_tileDownloader.get(tileKey);
			// TODO: We should put it into the cache once we downloaded it / directly implement blocking download functionality in cache
		}

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

	if (EndPage(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("EndPage problem"), TEXT("winmapviewer"), MB_OK);
		DeleteDC(hdcPrint);
		return;
	}

	if (EndDoc(hdcPrint) <= 0) {
		MessageBox(NULL, TEXT("EndDoc problem"), TEXT("winmapviewer"), MB_OK);
	}

	DeleteDC(hdcPrint);
}
