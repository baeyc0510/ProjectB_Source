#pragma once
#include <string>

// string -> wstring 변환
inline std::wstring ToWString(const std::string& str)
{
    return std::wstring(str.begin(), str.end());
}
