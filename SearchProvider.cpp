#include <iostream>
#include <msxml2.h>
#include <sstream>
#include <unistd.h>

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

std::wstring* SearchProvider::doQuery(std::wstring locationName) {
	// TODO: set up proxy and use proxy URL
	std::wstringstream strstr;
	strstr << "http://nominatim.openstreetmap.org/search?format=xml&limit=35&q=" << urlEncode(locationName);

	// TODO: Add user agent and explicit referer header
	HINTERNET hUrl = InternetOpenUrlW(m_hInternet, strstr.str().c_str(), NULL, 0, 0, 0);
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

	int length = MultiByteToWideChar(CP_UTF8, 0, rawResult.c_str(), rawResult.size(), 0, 0);
	std::wstring* result = new std::wstring(length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, rawResult.c_str(), rawResult.size(), &((*result)[0]), length);

	return result;
}

std::wstring getAttribute(IXMLDOMNamedNodeMap* attrs, const wchar_t* attributeName) {
	IXMLDOMNode* attr = NULL;
	attrs->getNamedItem(SysAllocString(attributeName), &attr);
	if (!attr) {
		throw "Missing attribute.";
	}
	BSTR rawValue;
	attr->get_text(&rawValue);
	std::wstring value(rawValue, SysStringLen(rawValue));
	SysFreeString(rawValue);
	attr->Release();

	return value;
}

std::vector<SearchResult> SearchProvider::search(std::wstring locationName) {
	std::vector<SearchResult> searchResult;

	std::wstring* rawXml = doQuery(locationName);

	CoInitialize(NULL);
	IXMLDOMDocument* doc = NULL;
	HRESULT hr = CoCreateInstance(CLSID_DOMDocument60, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&doc));

	if (FAILED(hr)) {
		throw "MSXML not available.";
	}

	doc->put_async(VARIANT_FALSE);
	doc->put_validateOnParse(VARIANT_FALSE);
	doc->put_resolveExternals(VARIANT_FALSE);

	VARIANT_BOOL ok = VARIANT_FALSE;

	BSTR b = SysAllocString(rawXml->c_str());
	doc->loadXML(b, &ok);
	SysFreeString(b);

	delete rawXml;

	if (ok != VARIANT_TRUE) {
		doc->Release();
		CoUninitialize();
		// throw "Failed to load XML.";
		return searchResult;
	}

	IXMLDOMNodeList* places = NULL;
	doc->selectNodes(SysAllocString(L"//place"), &places);

	long count = 0;
	places->get_length(&count);

	for (long i = 0; i < count; i++) {
		IXMLDOMNode* node = NULL;
		places->get_item(i, &node);

		IXMLDOMNamedNodeMap* attrs = NULL;
		node->get_attributes(&attrs);

		searchResult.push_back(
			SearchResult(
				getAttribute(attrs, L"display_name"),
				wcstod(getAttribute(attrs, L"lon").c_str(), NULL),
				wcstod(getAttribute(attrs, L"lat").c_str(), NULL)
			)
		);

		attrs->Release();
		node->Release();
	}

	doc->Release();
	CoUninitialize();

	return searchResult;
}
