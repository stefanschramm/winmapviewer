// Wrapper for Windows 95's ListView common control because it does not support wide character messages
class ListViewWrapper {
  public:
	static ListViewWrapper* create(HWND hwndListView);
	void insertColumn(int columnNumber, int width, std::string caption) const;
	virtual void insertItem(int item, int subItem, std::string utf8Text) const = 0;

  protected:
	ListViewWrapper(HWND hwndListView) : m_hwndListView(hwndListView) {}
	HWND m_hwndListView;
};

// ANSI variant
class ListViewWrapperA : public ListViewWrapper {
  public:
	ListViewWrapperA(HWND hwndListView) : ListViewWrapper(hwndListView) {}
	void insertItem(int item, int subItem, std::string utf8Text) const;
};

// Wide char variant
class ListViewWrapperW : public ListViewWrapper {
  public:
	ListViewWrapperW(HWND hwndListView) : ListViewWrapper(hwndListView) {}
	void insertItem(int item, int subItem, std::string utf8Text) const;
};
