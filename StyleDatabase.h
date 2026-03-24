struct Style {
	// Can't use const char* const in VC++6
	const char* url;
	const char* urlInsecure;
	const char* attributionText;
	const char* attributionLink;
};

class StyleDatabase {
  public:
	StyleDatabase(int identifierOffset) : m_identifierOffset(identifierOffset) {};
	const Style* get(int styleIdentifier) const;

  private:
	const int m_identifierOffset;
};
