#pragma once

#include <vector>
#include <wininet.h>

#include "Common.h"

class SearchResult {
  public:
	SearchResult(std::wstring displayName, double lon, double lat)
		: m_displayName(displayName), m_lonLat({lon, lat}) {
		  };

	std::wstring m_displayName;
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
