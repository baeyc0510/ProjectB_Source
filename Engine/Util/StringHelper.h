#pragma once
#include <string>

// string -> wstring 변환
inline std::wstring ToWString(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

inline std::wstring StringToWString(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}

// wstring -> string 변환
inline std::string WStringToString(const std::wstring& wstr)
{
    return std::string(wstr.begin(), wstr.end());
}
