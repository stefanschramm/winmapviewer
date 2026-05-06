#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

// VC++ 6 compatibility: Can't use const char* const
#if _MSC_VER == 1200
struct Style {
	char* url;
	char* urlInsecure;
	char* attributionText;
	char* attributionLink;
	int maxZoomLevel;
};
#else
struct Style {
	const char* const url;
	const char* const urlInsecure;
	const char* const attributionText;
	const char* const attributionLink;
	int maxZoomLevel;
};
#endif

class StyleDatabase {
  public:
	StyleDatabase(int identifierOffset) : m_identifierOffset(identifierOffset) {};
	const Style* get(int styleIdentifier) const;

  private:
	const int m_identifierOffset;
};
