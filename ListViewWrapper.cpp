// clang-format off
#include <windows.h>
#include <commctrl.h>
// clang-format on
#include <string>

#include "Common.h"
#include "Encoding.h"
#include "ListViewWrapper.h"

ListViewWrapper* ListViewWrapper::create(HWND hwndListView) {
	// TODO: return smart pointers?
	if (useUtf8()) {
		return new ListViewWrapperW(hwndListView);
	} else {
		return new ListViewWrapperA(hwndListView);
	}
}

void doInsertColumn(HWND hwndListView, int columnNumber, int width, std::string caption) {
	LVCOLUMNA col = {0};
	col.mask = LVCF_TEXT | LVCF_WIDTH;
	col.pszText = const_cast<char*>(caption.c_str());
	col.cx = width;
	SendMessageA(hwndListView, LVM_INSERTCOLUMNA, columnNumber, reinterpret_cast<LPARAM>(&col));
}

void ListViewWrapperA::insertColumn(int columnNumber, int width, std::string caption) {
	doInsertColumn(m_hwndListView, columnNumber, width, caption);
}

void ListViewWrapperW::insertColumn(int columnNumber, int width, std::string caption) {
	doInsertColumn(m_hwndListView, columnNumber, width, caption);
}

void ListViewWrapperA::insertItem(int item, int subItem, std::string utf8Text) {
	std::string result = convertUtf8ToCurrentCodepage(utf8Text);

	LVITEMA entry = {0};
	entry.mask = LVIF_TEXT;
	entry.iItem = item;
	entry.iSubItem = subItem;
	entry.pszText = const_cast<char*>(result.c_str());

	// Convention: We will fill the first field of a list entry first.
	if (subItem == 0) {
		SendMessageA(m_hwndListView, LVM_INSERTITEMA, 0, reinterpret_cast<LPARAM>(&entry));
	} else {
		SendMessageA(m_hwndListView, LVM_SETITEMA, 0, reinterpret_cast<LPARAM>(&entry));
	}
}

void ListViewWrapperW::insertItem(int item, int subItem, std::string utf8Text) {
	std::wstring result = convertUtf8ToWide(utf8Text);

	LVITEMW entry = {0};
	entry.mask = LVIF_TEXT;
	entry.iItem = item;
	entry.iSubItem = subItem;
	entry.pszText = const_cast<wchar_t*>(result.c_str());

	// Convention: We will fill the first field of a list entry first.
	if (subItem == 0) {
		SendMessageW(m_hwndListView, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&entry));
	} else {
		SendMessageW(m_hwndListView, LVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&entry));
	}
}
