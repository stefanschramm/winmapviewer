#pragma once

// Disable long identifiers warning
#pragma warning(disable : 4786)

#include <vector>
#include <windows.h>
#include <wininet.h>

#include "Common.h"

struct SearchResult {
	std::wstring m_displayName;
	std::wstring m_osmType;
	std::wstring m_osmId;
	std::wstring m_class;
	std::wstring m_type;
	LonLat m_lonLat;
};

class SearchProvider {
  public:
	SearchProvider();
	~SearchProvider();
	std::vector<SearchResult> search(std::wstring locationName, std::vector<SearchResult> searchResults) const;

  private:
	std::string doQuery(std::wstring locationName) const;

	HINTERNET m_hInternet;
};
