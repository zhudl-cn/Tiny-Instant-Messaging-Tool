#include "utf8.h"

namespace Convertor {
	std::string unicode_to_utf8(std::string& unicode_str) {
		std::string utf8_str = "";
		utf8::utf16to8(unicode_str.begin(), unicode_str.end(), std::back_inserter(utf8_str));
		return utf8_str;
	}

	std::string utf8_to_unicode(const std::string& utf8_str) {
		std::string unicode_str;
		utf8::utf8to16(utf8_str.begin(), utf8_str.end(), std::back_inserter(unicode_str));
		return unicode_str;
	}
}