#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

struct Style {
	// Can't use const char* const in VC++6
	char* url;
	char* urlInsecure;
	char* attributionText;
	char* attributionLink;
	int maxZoomLevel;
};

class StyleDatabase {
  public:
	StyleDatabase(int identifierOffset) : m_identifierOffset(identifierOffset) {};
	const Style* get(int styleIdentifier) const;

  private:
	const int m_identifierOffset;
};
