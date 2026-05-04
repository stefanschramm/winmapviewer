#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <vector>
#include <windows.h>
#include <wininet.h>

#include "Common.h"

struct SearchResult {
	std::string m_displayName;
	std::string m_osmType;
	std::string m_osmId;
	std::string m_class;
	std::string m_type;
	LonLat m_lonLat;
};

class SearchProvider {
  public:
	SearchProvider();
	~SearchProvider();
	std::vector<SearchResult> search(std::string locationNameUtf8, std::vector<SearchResult> searchResults) const;

  private:
	std::string doQuery(std::string locationNameUtf8) const;

	HINTERNET m_hInternet;
};
