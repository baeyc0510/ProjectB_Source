#pragma once

// A와 B가 동일한지 (epsilon 오차 허용) 
inline bool IsNearlyEqual(float a, float b, float epsilon = 1.e-4f)
{
    return std::abs(a - b) <= epsilon; 
}

inline bool IsNearlyZero(float value)
{
    return IsNearlyEqual(value, 0.0f);
}