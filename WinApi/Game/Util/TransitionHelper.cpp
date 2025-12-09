#include "pch.h"
#include "TransitionHelper.h"

ESceneType TransitionHelper::ToSceneType(const wstring& str)
{
    if (str == L"Title")	return ESceneType::Title;
    if (str == L"Stage01")	return ESceneType::Stage01;
    if (str == L"Stage02")	return ESceneType::Stage02;
    if (str == L"Stage03")	return ESceneType::Stage03;
    if (str == L"Stage04")	return ESceneType::Stage04;
    if (str == L"Stage05")	return ESceneType::Stage05;
    if (str == L"Stage_Boss01") return ESceneType::Stage_Boss01;
    // 정의되지 않은 씬 이름
    assert(false && "Unknown Stage name");
    return ESceneType::Title;
}
