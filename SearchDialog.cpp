// windows.h is required to be included *before* commctrl.h
// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <sstream>
#include <string>
#include <windowsx.h>

#include "Common.h"
#include "Encoding.h"
#include "ListViewWrapper.h"
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

	ListViewWrapper* wrapper = ListViewWrapper::create(m_hwndListView);

	wrapper->insertColumn(0, 350, "Name");
	wrapper->insertColumn(1, 65, "Lat");
	wrapper->insertColumn(2, 65, "Lon");
	wrapper->insertColumn(3, 65, "Class");
	wrapper->insertColumn(4, 65, "Type");
	wrapper->insertColumn(5, 65, "OSM Type");
	wrapper->insertColumn(6, 65, "OSM ID");

	// TODO: use smart pointer
	delete wrapper;

	updateResultList();

	return TRUE;
}

std::string getLocationName(HWND hInputField) {
	if (useUtf8()) {
		std::wstring locationNameWide;
		int length = GetWindowTextLengthW(hInputField) + 1;
		locationNameWide.resize(length);
		GetWindowTextW(hInputField, &locationNameWide[0], length);
		std::string locationNameUtf8 = convertWideToUtf8(locationNameWide);
		return locationNameUtf8;
	} else {
		std::string locationName;
		int length = GetWindowTextLengthA(hInputField) + 1;
		locationName.resize(length);
		GetWindowTextA(hInputField, &locationName[0], length);
		std::string locationNameUtf8 = convertCurrentCodepageToUtf8(locationName);
		return locationNameUtf8;
	}
}

void SearchDialog::ok() {
	ListView_DeleteAllItems(m_hwndListView);
	m_searchResults.clear();

	std::string locationName = getLocationName(GetDlgItem(m_hwndDialog, IDC_DLG_LOCATIONNAME));

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

	ListViewWrapper* wrapper = ListViewWrapper::create(m_hwndListView);

	for (std::vector<SearchResult>::iterator it = m_searchResults.begin(); it != m_searchResults.end(); ++it) {
		std::stringstream latstrstr;
		latstrstr << it->m_lonLat.lat;

		std::stringstream lonstrstr;
		lonstrstr << it->m_lonLat.lon;

		wrapper->insertItem(i, 0, it->m_displayName);
		wrapper->insertItem(i, 1, latstrstr.str());
		wrapper->insertItem(i, 2, lonstrstr.str());
		wrapper->insertItem(i, 3, it->m_class);
		wrapper->insertItem(i, 4, it->m_type);
		wrapper->insertItem(i, 5, it->m_osmType);
		wrapper->insertItem(i, 6, it->m_osmId);

		i++;
	}

	// TODO: use smart pointer
	delete wrapper;
}

void SearchDialog::selectItem() {
	int row = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);
	if (row >= 0) {
		SearchResult selectedResult = m_searchResults.at(row);
		SendMessage(m_hwndMain, WM_USER_SEARCH_SET_LONLAT, 0, reinterpret_cast<LPARAM>(&selectedResult.m_lonLat));
	}
}
