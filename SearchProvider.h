#pragma once

#include <vector>
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
	std::vector<SearchResult> search(std::wstring locationName);

  private:
	std::wstring* doQuery(std::wstring locationName);

	HINTERNET m_hInternet;
};
