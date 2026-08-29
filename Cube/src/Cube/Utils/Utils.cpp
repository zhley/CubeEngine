#include "Utils.h"

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace Cube {

void Utils::setConsoleUtf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

// append a utf32 code point to a utf8 string
static void appendUtf8(std::string& out, char32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

// read a utf8 code point from a utf8 string
static size_t readUtf8(std::string_view s, size_t pos, char32_t& cp) {
    unsigned char c = static_cast<unsigned char>(s[pos]);
    if (c < 0x80) {
        cp = c;
        return 1;
    } else if ((c >> 5) == 0x06) {  // 110xxxxx
        cp = (c & 0x1F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F);
        return 2;
    } else if ((c >> 4) == 0x0E) {  // 1110xxxx
        cp = (c & 0x0F) << 12;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 2]) & 0x3F);
        return 3;
    } else {  // 11110xxx
        cp = (c & 0x07) << 18;
        cp |= (static_cast<unsigned char>(s[pos + 1]) & 0x3F) << 12;
        cp |= (static_cast<unsigned char>(s[pos + 2]) & 0x3F) << 6;
        cp |= (static_cast<unsigned char>(s[pos + 3]) & 0x3F);
        return 4;
    }
}

// TODO: error handling

std::u16string Utils::utf8To16(std::string_view utf8) {
    std::u16string out;
    char32_t cp = 0;
    for (size_t i = 0; i < utf8.size();) {
        size_t len = readUtf8(utf8, i, cp);
        i += len;
        if (cp <= 0xFFFF) {
            out.push_back(static_cast<char16_t>(cp));
        } else {
            // surrogate pair
            cp -= 0x10000;
            out.push_back(static_cast<char16_t>(0xD800 | (cp >> 10)));
            out.push_back(static_cast<char16_t>(0xDC00 | (cp & 0x3FF)));
        }
    }
    return out;
}

std::string Utils::utf16To8(std::u16string_view utf16) {
    std::string out;
    char32_t cp = 0;
    for (size_t i = 0; i < utf16.size(); ++i) {
        char16_t c = utf16[i];
        if (c >= 0xD800 && c <= 0xDBFF) {
            // surrogate pair
            char16_t low = utf16[i + 1];
            cp = ((c - 0xD800) << 10) | (low - 0xDC00);
            cp += 0x10000;
            ++i;
        } else {
            cp = c;
        }
        appendUtf8(out, cp);
    }
    return out;
}

std::u32string Utils::utf8To32(std::string_view utf8) {
    std::u32string out;
    char32_t cp = 0;
    for (size_t i = 0; i < utf8.size();) {
        size_t len = readUtf8(utf8, i, cp);
        i += len;
        out.push_back(cp);
    }
    return out;
}

std::string Utils::utf32To8(std::u32string_view utf32) {
    std::string out;
    for (char32_t cp : utf32) {
        appendUtf8(out, cp);
    }
    return out;
}

}  // namespace Cube