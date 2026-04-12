#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <wtypes.h>

#include "SearchProvider.h"

class SearchDialog {
  public:
	SearchDialog(HINSTANCE hInst, HWND hWnd);
	~SearchDialog();
	static LRESULT CALLBACK wndProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

  private:
	SearchProvider* m_searchProvider;
	// Not sure why I need to store it and why GetParent(hDlg) does not return this hWnd...
	HWND m_hwndMain;
	HWND m_hwndDialog;
	HWND m_hwndListView;
	std::vector<SearchResult> m_searchResults;
	SearchResult m_clickedSearchResult;

	void init(HWND hwndDialog);
	void ok();
	void cancel();
	void openInOsm();
	void selectItem();
	void updateResultList();
	LRESULT CALLBACK wndProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
};
