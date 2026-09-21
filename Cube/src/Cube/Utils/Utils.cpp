#include "Utils.h"
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <iconv.h>
#include <cerrno>
#endif

#include <string_view>


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

// use system API
std::string Utils::wcharToUtf8(std::wstring_view wstr) {
    if (wstr.empty()) return std::string();
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), out.data(), len, nullptr, nullptr);
    return out;
#else
    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
    if (cd == (iconv_t)-1) return std::string();

    std::string out;
    out.resize(wstr.size() * 4);

    char* inBuf = reinterpret_cast<char*>(const_cast<wchar_t*>(wstr.data()));
    size_t inBytes = wstr.size() * sizeof(wchar_t);

    char* outBuf = out.data();
    size_t outBytes = out.size();

    size_t ret = iconv(cd, &inBuf, &inBytes, &outBuf, &outBytes);
    iconv_close(cd);

    if (ret == (size_t)-1) return std::string();

    out.resize(out.size() - outBytes);
    return out;
#endif
}

std::wstring Utils::utf8ToWchar(std::string_view str) {
    if (str.empty()) return std::wstring();
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
    if (wlen <= 0) return std::wstring();
    std::wstring wout(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), wout.data(), wlen);
    return wout;
#else
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1) return std::wstring();

    std::wstring wout;
    wout.resize(utf8.size());

    char* inBuf = const_cast<char*>(utf8.data());
    size_t inBytes = utf8.size();

    char* outBuf = reinterpret_cast<char*>(wout.data());
    size_t outBytes = wout.size() * sizeof(wchar_t);

    size_t ret = iconv(cd, &inBuf, &inBytes, &outBuf, &outBytes);
    iconv_close(cd);

    if (ret == (size_t)-1) return std::wstring();

    wout.resize((wout.size() * sizeof(wchar_t) - outBytes) / sizeof(wchar_t));
    return wout;
#endif
}

}  // namespace Cube