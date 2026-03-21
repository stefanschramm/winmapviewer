// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <iostream>
#include <sstream>
#include <string>
#include <windowsx.h>

#include "Common.h"
#include "SearchDialog.h"
#include "SearchProvider.h"
#include "resource.h"

#define IDM_OPEN_IN_OSM 10002

HWND hMainWindow = NULL;

SearchProvider* searchProvider = NULL;

// TODO: ownership? initialization?
std::vector<SearchResult> searchResult;

SearchResult clickedSearchResult;

void updateResultList(HWND hListView) {
	int i = 0;
	for (std::vector<SearchResult>::iterator it = searchResult.begin(); it != searchResult.end(); ++it) {
		LVITEMW entry = {0};
		entry.mask = LVIF_TEXT;
		entry.iItem = i++;

		entry.iSubItem = 0;
		entry.pszText = const_cast<wchar_t*>(it->m_displayName.c_str());
		SendMessageW(hListView, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		std::wstringstream latText;
		latText << it->m_lonLat.lat;
		entry.iSubItem = 1;
		entry.pszText = const_cast<wchar_t*>(latText.str().c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		std::wstringstream lonText;
		lonText << it->m_lonLat.lon;
		entry.iSubItem = 2;
		entry.pszText = const_cast<wchar_t*>(lonText.str().c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 3;
		entry.pszText = const_cast<wchar_t*>(it->m_class.c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 4;
		entry.pszText = const_cast<wchar_t*>(it->m_type.c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 5;
		entry.pszText = const_cast<wchar_t*>(it->m_osmType.c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 6;
		entry.pszText = const_cast<wchar_t*>(it->m_osmId.c_str());
		SendMessageW(hListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));
	}
}

void selectItem(LPNMITEMACTIVATE item) {
	int row = item->iItem;
	int col = item->iSubItem;
	if (row >= 0) {
		SearchResult selectedResult = searchResult.at(row);
		SendMessage(hMainWindow, WM_SEARCH_SET_LONLAT, 0, reinterpret_cast<LPARAM>(&selectedResult.m_lonLat));
	}
}

LRESULT CALLBACK SearchDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hListView = GetDlgItem(hDlg, IDC_SEARCH_RESULTS);

	switch (message) {
		case WM_INITDIALOG: {
			if (searchProvider == NULL) {
				searchProvider = new SearchProvider();
			}

			LVCOLUMNW col = {0};
			col.mask = LVCF_TEXT | LVCF_WIDTH;

			col.pszText = const_cast<wchar_t*>(L"Name");
			col.cx = 350;
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Lat");
			col.cx = 65;
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Lon");
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Class");
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 3, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Type");
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 4, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"OSM Type");
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 5, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"OSM ID");
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 6, reinterpret_cast<LPARAM>(&col));

			updateResultList(hListView);

			return TRUE;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				ListView_DeleteAllItems(hListView);

				HWND hInputField = GetDlgItem(hDlg, IDC_DLG_LOCATIONNAME);
				int length = GetWindowTextLength(hInputField) + 1;
				std::wstring locationName;
				locationName.resize(length);
				GetWindowTextW(hInputField, &locationName[0], length);

				// TODO: do we have to free the previous searchResult?
				searchResult = searchProvider->search(locationName);
				updateResultList(hListView);

				return TRUE;
			}
			if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hDlg, LOWORD(wParam));

				delete searchProvider;
				searchProvider = NULL;

				return TRUE;
			}
			if (LOWORD(wParam) == IDM_OPEN_IN_OSM) {
				std::stringstream url;
				url << "https://www.openstreetmap.org/"
					<< urlEncode(clickedSearchResult.m_osmType)
					<< "/"
					<< urlEncode(clickedSearchResult.m_osmId);
				ShellExecute(
					NULL,
					"open",
					url.str().c_str(),
					NULL,
					NULL,
					SW_SHOWNORMAL
				);
			}
			break;

		case WM_CONTEXTMENU: {
			POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

			LVHITTESTINFO hitTest = {0};
			hitTest.pt = point;
			ScreenToClient(hListView, &hitTest.pt);
			int index = ListView_HitTest(hListView, &hitTest);

			if (index != -1 && (hitTest.flags & LVHT_ONITEM)) {
				clickedSearchResult = searchResult.at(index);

				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDM_OPEN_IN_OSM, "Open OSM object in browser");
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
				DestroyMenu(hMenu);
			}

			break;
		}

		case WM_NOTIFY:
			LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
			if (hdr->hwndFrom == hListView && hdr->code == LVN_ITEMACTIVATE) {
				selectItem((LPNMITEMACTIVATE)lParam);
			}
			break;
	}
	return FALSE;
}

void search(HINSTANCE hInst, HWND hWnd) {
	// Not sure why I need to store it and why GetParent(hDlg) does not return this hWnd...
	hMainWindow = hWnd;
	DialogBox(hInst, (LPCTSTR)IDD_SEARCH, hWnd, (DLGPROC)SearchDialog);
}
