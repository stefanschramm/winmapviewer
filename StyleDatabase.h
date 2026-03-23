struct Style {
	const char* const url;
	const char* const urlInsecure;
	const char* const attributionText;
	const char* const attributionLink;
};

class StyleDatabase {
  public:
	StyleDatabase(int identifierOffset) : m_identifierOffset(identifierOffset) {};
	const Style* get(int styleIdentifier) const;

  private:
	const int m_identifierOffset;
};
