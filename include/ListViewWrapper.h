// Wrapper for Windows 95's ListView common control because it does not support wide character messages
class ListViewWrapper {
  public:
	static ListViewWrapper* create(HWND hwndListView);
	virtual void insertColumn(int columnNumber, int width, std::string caption) = 0;
	virtual void insertItem(int item, int subItem, std::string utf8Text) = 0;
};

class ListViewWrapperA : public ListViewWrapper {
  public:
	ListViewWrapperA(HWND hwndListView) : m_hwndListView(hwndListView) {}
	void insertColumn(int columnNumber, int width, std::string caption);
	void insertItem(int item, int subItem, std::string utf8Text);

  private:
	HWND m_hwndListView;
};

class ListViewWrapperW : public ListViewWrapper {
  public:
	ListViewWrapperW(HWND hwndListView) : m_hwndListView(hwndListView) {}
	void insertColumn(int columnNumber, int width, std::string caption);
	void insertItem(int item, int subItem, std::string utf8Text);

  private:
	HWND m_hwndListView;
};
