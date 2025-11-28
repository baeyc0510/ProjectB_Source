#pragma once
#include "Game/Enum.h"

class CAnimator;
class CAbilitySystem;

class AnimEventHelper
{
public:
	// 애니메이션 이벤트 문자열 -> EGameEvent 변환 (정의되지 않은 이름은 assert)
	static EGameEvent ToGameEvent(const wstring& str);

	// Animator의 OnFrameEvent를 AbilitySystem으로 연결
	static void ConnectAbilitySystem(CAnimator* animator, CAbilitySystem* abilitySystem);
};
