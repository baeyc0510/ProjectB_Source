#pragma once
#include "Game/Enum.h"

class Animator;
class AbilitySystem;

class AnimationHelper
{
public:
	// 애니메이션 이벤트 문자열 -> EGameEvent 변환 (정의되지 않은 이름은 assert)
	static EGameEvent ToGameEvent(const wstring& str);

	// Animator의 OnFrameEvent를 AbilitySystem으로 연결
	static void ConnectAbilitySystem(Animator* animator, AbilitySystem* abilitySystem);
	
	// 애니메이션 추가 헬퍼
	static void AddAnimation(Animator* animator, const wstring& aniName, const wstring& path, bool bShouldRepeat);
};
