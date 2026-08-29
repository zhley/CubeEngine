#pragma once

#include <string>
#include <string_view>

namespace Cube {

namespace Utils {

void setConsoleUtf8();

// code conversion
std::u16string utf8To16(std::string_view utf8);
std::string utf16To8(std::u16string_view utf16);
std::u32string utf8To32(std::string_view utf8);
std::string utf32To8(std::u32string_view utf32);

};

}