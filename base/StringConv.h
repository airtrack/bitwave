#ifndef STRING_CONV_H
#define STRING_CONV_H

#include <assert.h>
#include <string>
#include <vector>
#include <Windows.h>

inline std::wstring UTF8ToUnicode(const std::string& str)
{
    int count = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), 0, 0);
    assert(count > 0);
    std::vector<wchar_t> buffer(count);
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), &buffer[0], count);
    return std::wstring(&buffer[0], count);
}

inline std::string UnicodeToUTF8(const std::wstring& wstr)
{
    int count = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), 0, 0, 0, 0);
    assert(count > 0);
    std::vector<char> buffer(count);
    ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), &buffer[0], count, 0, 0);
    return std::string(&buffer[0], count);
}

#endif // STRING_CONV_H
