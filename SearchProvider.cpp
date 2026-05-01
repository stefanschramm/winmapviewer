#include <sstream>

#define XML_H_IMPLEMENTATION
#include "lib/xml/xml.h"

#include "Common.h"
#include "SearchProvider.h"

SearchProvider::SearchProvider() {
	m_hInternet = InternetOpen(TEXT("winmapviewer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!m_hInternet) {
		throw "Unable to initialize WinINet.";
	}
}

SearchProvider::~SearchProvider() {
	InternetCloseHandle(m_hInternet);
}

std::string SearchProvider::doQuery(std::wstring locationName) const {
	std::stringstream strstr;
	// Reverse proxy server URL is used to be able to centrally disable/change usage if required.
	// TODO: add option to (not) use TLS
	strstr << "http://osm.kesto.de/nominatim/search?format=xml&limit=35&q=" << urlEncode(locationName);

	HINTERNET hUrl = InternetOpenUrl(m_hInternet, strstr.str().c_str(), NULL, 0, 0, 0);
	if (!hUrl) {
		throw "Unable to do nominatim request.";
	}

	std::string rawResult;

	char buffer[10240];
	DWORD bytesRead = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		rawResult.append(buffer, bytesRead);
	}
	InternetCloseHandle(hUrl);

	return rawResult;
}

std::wstring getAttribute(XMLNode* place, const char* attributeName) {
	const char* value = xml_node_attr(place, attributeName);

	int size = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, value, -1, &result[0], size);

	return result;
}

std::vector<SearchResult> SearchProvider::search(std::wstring locationName, std::vector<SearchResult> searchResults) const {
	// Result XML looks like this:
	// <?xml version="1.0" encoding="UTF-8" ?>
	// <searchresults timestamp="Fri, 01 May 2026 19:15:01 +00:00" attribution="Data © OpenStreetMap contributors, ODbL 1.0. http://osm.org/copyright" querystring="Berlin" more_url="https://osm.kesto.de, nominatim.openstreetmap.org/search?q=Berlin&amp;addressdetails=1&amp;limit=45&amp;exclude_place_ids=R62422&amp;format=xml" exclude_place_ids="R62422">
	//   <place place_id="134060781" osm_type="relation" osm_id="62422" ref="BE" lat="52.5173885" lon="13.3951309" boundingbox="52.3382448,52.6755087,13.0883450,13.7611609" place_rank="8" address_rank="16" display_name="Berlin, Deutschland" class="boundary" type="administrative" importance="0.8522196536088086" />
	// </searchresults>

	XMLNode* root = xml_parse_string(doQuery(locationName).c_str());
	if (root == NULL) {
		throw "Unable to parse result XML.";
	}

	XMLNode* entries = xml_node_child_at(root, 0);
	for (size_t i = 0; i < entries->children->len; i++) {
		XMLNode* place = xml_node_child_at(entries, i);

		LonLat lonLat = {
			wcstod(getAttribute(place, "lon").c_str(), NULL),
			wcstod(getAttribute(place, "lat").c_str(), NULL)
		};

		SearchResult searchResult;
		searchResult.m_displayName = getAttribute(place, "display_name");
		searchResult.m_osmType = getAttribute(place, "osm_type");
		searchResult.m_osmId = getAttribute(place, "osm_id");
		searchResult.m_class = getAttribute(place, "class");
		searchResult.m_type = getAttribute(place, "type");
		searchResult.m_lonLat = lonLat;

		searchResults.push_back(searchResult);
	}

	xml_node_free(root);

	return searchResults;
}
