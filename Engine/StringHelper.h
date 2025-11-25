#pragma once
#include <string>


// string -> wstring º¯È¯
std::wstring ToWString(const std::string& str) 
{
    return std::wstring(str.begin(), str.end());
}