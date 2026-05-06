#include <windows.h>

#include "Encoding.h"

bool useUtf8() {
	OSVERSIONINFO osvi = {0};
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);

	return osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

std::string urlEncode(const std::string& value) {
	static const char hex[] = "0123456789ABCDEF";

	std::string encoded;
	for (size_t i = 0; i < value.size(); ++i) {
		unsigned char c = value[i];

		if ((c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') ||
			c == '-' || c == '_' || c == '.' || c == '~') {
			encoded += c;
		} else {
			encoded += '%';
			encoded += hex[c >> 4];
			encoded += hex[c & 0x0F];
		}
	}

	return encoded;
}

std::string convertCurrentCodepageToUtf8(const std::string& text) {
	// TODO: implement
	return text;
}

std::string convertWideToUtf8(const std::wstring& wtext) {
	const wchar_t* input = wtext.c_str();

	int size = WideCharToMultiByte(CP_UTF8, 0, input, -1, NULL, 0, NULL, NULL);
	std::string result(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, input, -1, &result[0], size, NULL, NULL);

	return result;
}

std::string convertUtf8ToCurrentCodepage(const std::string& utf8Text) {
	// For now just remove all non-ASCII characters.
	// TODO: Implement mapping from UTF-8 to current active codepage

	int size = 0;

	for (int i = 0; i < utf8Text.size(); i++) {
		if (!(utf8Text[i] & 0x80)) {
			size++;
		}
	}

	std::string result(size, 0);

	int o = 0;
	for (int j = 0; j < utf8Text.size(); j++) {
		if (!(utf8Text[j] & 0x80)) {
			result[o] = utf8Text[j];
			o++;
		}
	}

	return result;
}

std::wstring convertUtf8ToWide(const std::string& utf8Text) {
	const char* utf8Input = utf8Text.c_str();

	int size = MultiByteToWideChar(CP_UTF8, 0, utf8Input, -1, NULL, 0);
	std::wstring result(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Input, -1, &result[0], size);

	return result;
}
