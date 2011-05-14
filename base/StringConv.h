#ifndef STRING_CONV_H
#define STRING_CONV_H

#include <assert.h>
#include <string>
#include <vector>
#include <Windows.h>

inline std::wstring MultiByteToUnicode(const std::string& str, unsigned code_page)
{
    int count = ::MultiByteToWideChar(code_page, 0, str.c_str(), str.size(), 0, 0);
    assert(count > 0);
    std::vector<wchar_t> buffer(count);
    ::MultiByteToWideChar(code_page, 0, str.c_str(), str.size(), &buffer[0], count);
    return std::wstring(&buffer[0], count);
}

inline std::string UnicodeToMultiByte(const std::wstring& wstr, unsigned code_page)
{
    int count = ::WideCharToMultiByte(code_page, 0, wstr.c_str(), wstr.size(), 0, 0, 0, 0);
    assert(count > 0);
    std::vector<char> buffer(count);
    ::WideCharToMultiByte(code_page, 0, wstr.c_str(), wstr.size(), &buffer[0], count, 0, 0);
    return std::string(&buffer[0], count);
}

inline std::wstring UTF8ToUnicode(const std::string& str)
{
    return MultiByteToUnicode(str, CP_UTF8);
}

inline std::string UnicodeToUTF8(const std::wstring& wstr)
{
    return UnicodeToMultiByte(wstr, CP_UTF8);
}

inline std::wstring ANSIToUnicode(const std::string str)
{
    return MultiByteToUnicode(str, CP_ACP);
}

inline std::string UnicodeToANSI(const std::wstring& wstr)
{
    return UnicodeToMultiByte(wstr, CP_ACP);
}

inline std::string ANSIToUTF8(const std::string& ansi)
{
    return UnicodeToUTF8(ANSIToUnicode(ansi));
}

inline std::string UTF8ToANSI(const std::string& utf8)
{
    return UnicodeToANSI(UTF8ToUnicode(utf8));
}

#endif // STRING_CONV_H
