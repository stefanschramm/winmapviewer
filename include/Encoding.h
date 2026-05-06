#include <string>

bool useUtf8();

std::string urlEncode(const std::string& url);

std::string convertCurrentCodepageToUtf8(const std::string& text);

std::string convertWideToUtf8(const std::wstring& wtext);

std::string convertUtf8ToCurrentCodepage(const std::string& utf8Text);

std::wstring convertUtf8ToWide(const std::string& utf8Text);
