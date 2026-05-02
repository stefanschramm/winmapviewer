// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <sstream>
#include <string>
#include <windowsx.h>

#include "Common.h"
#include "SearchDialog.h"
#include "resource.h"

#define IDM_OPEN_IN_OSM 10002

SearchDialog::SearchDialog(HINSTANCE hInstance, HWND hWnd, const SearchProvider& searchProvider)
	: m_hInstance(hInstance),
	  m_hwndMain(hWnd),
	  m_searchProvider(searchProvider) {
}

void SearchDialog::show() {
	DialogBoxParam(
		m_hInstance,
		reinterpret_cast<LPCTSTR>(IDD_SEARCH),
		m_hwndMain,
		reinterpret_cast<DLGPROC>(SearchDialog::wndProcStatic),
		reinterpret_cast<LPARAM>(this)
	);
}

LRESULT CALLBACK SearchDialog::wndProcStatic(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	SearchDialog* self;

	try {
		if (message == WM_INITDIALOG) {
			// Store pointer to SearchDialog instance
			self = reinterpret_cast<SearchDialog*>(lParam);
			SetWindowLongPtr(hDialog, GWLP_USERDATA, lParam);
			return self->init(hDialog);
		}

		self = reinterpret_cast<SearchDialog*>(GetWindowLongPtr(hDialog, GWLP_USERDATA));

		return self->wndProc(hDialog, message, wParam, lParam);
	} catch (char const* e) {
		panicMessage("search dialog procedure", e);
		exit(EXIT_FAILURE);
	}
}

LRESULT CALLBACK SearchDialog::wndProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				ok();
				return FALSE;
			}
			if (LOWORD(wParam) == IDCANCEL) {
				cancel();
				return FALSE;
			}
			if (LOWORD(wParam) == IDM_OPEN_IN_OSM) {
				openInOsm();
				return FALSE;
			}
			break;

		case WM_CONTEXTMENU: {
			POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

			LVHITTESTINFO hitTest = {0};
			hitTest.pt = point;
			ScreenToClient(m_hwndListView, &hitTest.pt);
			int index = ListView_HitTest(m_hwndListView, &hitTest);

			if (index != -1 && (hitTest.flags & LVHT_ONITEM)) {
				m_clickedSearchResult = m_searchResults.at(index);

				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDM_OPEN_IN_OSM, "Open OSM object in browser");
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hDialog, NULL);
				DestroyMenu(hMenu);
			}
			break;
		}

		case WM_NOTIFY: {
			LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
			if (hdr->hwndFrom == m_hwndListView && hdr->code == LVN_ITEMCHANGED) {
				selectItem();
			}
			break;
		}
	}
	return FALSE;
}

BOOL SearchDialog::init(HWND hwndDialog) {
	m_hwndDialog = hwndDialog;
	m_hwndListView = GetDlgItem(hwndDialog, IDC_SEARCH_RESULTS);

	LVCOLUMNW col = {0};
	col.mask = LVCF_TEXT | LVCF_WIDTH;

	col.pszText = const_cast<wchar_t*>(L"Name");
	col.cx = 350;
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 0, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"Lat");
	col.cx = 65;
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 1, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"Lon");
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 2, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"Class");
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 3, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"Type");
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 4, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"OSM Type");
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 5, reinterpret_cast<LPARAM>(&col));

	col.pszText = const_cast<wchar_t*>(L"OSM ID");
	SendMessageW(m_hwndListView, LVM_INSERTCOLUMNW, 6, reinterpret_cast<LPARAM>(&col));

	updateResultList();

	return TRUE;
}

void SearchDialog::ok() {
	ListView_DeleteAllItems(m_hwndListView);
	m_searchResults.clear();

	HWND hInputField = GetDlgItem(m_hwndDialog, IDC_DLG_LOCATIONNAME);
	int length = GetWindowTextLength(hInputField) + 1;
	std::wstring locationName;
	locationName.resize(length);
	GetWindowTextW(hInputField, &locationName[0], length);

	// TODO: Do search in a thread
	m_searchResults = m_searchProvider.search(locationName, m_searchResults);
	updateResultList();

	SetFocus(GetDlgItem(m_hwndDialog, IDC_SEARCH_RESULTS));
}

void SearchDialog::cancel() {
	EndDialog(m_hwndDialog, IDCANCEL);
}

void SearchDialog::openInOsm() {
	std::stringstream url;

	url << "https://www.openstreetmap.org/"
		<< urlEncode(m_clickedSearchResult.m_osmType)
		<< "/"
		<< urlEncode(m_clickedSearchResult.m_osmId);

	ShellExecute(
		NULL,
		"open",
		url.str().c_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}

void SearchDialog::updateResultList() {
	int i = 0;
	for (std::vector<SearchResult>::iterator it = m_searchResults.begin(); it != m_searchResults.end(); ++it) {
		LVITEMW entry = {0};
		entry.mask = LVIF_TEXT;
		entry.iItem = i++;

		entry.iSubItem = 0;
		entry.pszText = const_cast<wchar_t*>(it->m_displayName.c_str());
		SendMessageW(m_hwndListView, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		// VC++ 6 compatibility: Don't use std::wstringstream for the next two because it is broken in debug builds.
		wchar_t tmp[16];

		swprintf(tmp, L"%.7f", it->m_lonLat.lat);
		entry.iSubItem = 1;
		entry.pszText = tmp;
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		swprintf(tmp, L"%.7f", it->m_lonLat.lon);
		entry.iSubItem = 2;
		entry.pszText = tmp;
		SendMessageA(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 3;
		entry.pszText = const_cast<wchar_t*>(it->m_class.c_str());
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 4;
		entry.pszText = const_cast<wchar_t*>(it->m_type.c_str());
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 5;
		entry.pszText = const_cast<wchar_t*>(it->m_osmType.c_str());
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));

		entry.iSubItem = 6;
		entry.pszText = const_cast<wchar_t*>(it->m_osmId.c_str());
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));
	}
}

void SearchDialog::selectItem() {
	int row = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);
	if (row >= 0) {
		SearchResult selectedResult = m_searchResults.at(row);
		SendMessage(m_hwndMain, WM_USER_SEARCH_SET_LONLAT, 0, reinterpret_cast<LPARAM>(&selectedResult.m_lonLat));
	}
}
