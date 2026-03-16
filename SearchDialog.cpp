// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <iostream>
#include <sstream>
#include <string>

#include "Common.h"
#include "SearchDialog.h"
#include "SearchProvider.h"
#include "resource.h"

HWND hMainWindow = NULL;

SearchProvider* searchProvider = NULL;

// TODO: ownership? initialization?
std::vector<SearchResult> searchResult;

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
	}

	SendMessageW(hListView, LVM_SETCOLUMNWIDTH, 0, 350);
	SendMessageW(hListView, LVM_SETCOLUMNWIDTH, 1, 50);
	SendMessageW(hListView, LVM_SETCOLUMNWIDTH, 2, 50);
}

void selectItem(LPNMITEMACTIVATE item) {
	int row = item->iItem;
	int col = item->iSubItem;
	if (row >= 0) {
		SearchResult selectedResult = searchResult.at(row);
		SendMessage(hMainWindow, WM_SEARCH_SET_LONLAT, 0, (LPARAM)&selectedResult.m_lonLat);
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
			col.cx = 120;
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Lat");
			col.cx = 30;
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&col));

			col.pszText = const_cast<wchar_t*>(L"Lon");
			col.cx = 30;
			SendMessageW(hListView, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&col));

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
			break;
		case WM_NOTIFY:
			LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
			if (hdr->hwndFrom == hListView && hdr->code == LVN_ITEMACTIVATE) {
				selectItem(reinterpret_cast<LPNMITEMACTIVATE>(lParam));
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
