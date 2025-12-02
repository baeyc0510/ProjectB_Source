#pragma once
#include "Game/Enum.h"

class TransitionHelper
{
public:
    // 씬 이름 문자열 -> 씬 타입 변환
    static ESceneType ToSceneType(const wstring& str);
};
