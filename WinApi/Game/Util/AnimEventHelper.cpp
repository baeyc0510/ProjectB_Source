#include "pch.h"
#include "AnimEventHelper.h"
#include "Game/Component/AbilitySystem.h"

EGameEvent AnimEventHelper::ToGameEvent(const wstring& str)
{
	if (str == L"HitCheck")			return EGameEvent::HitCheck;
	if (str == L"ComboWindowOpen")	return EGameEvent::ComboWindowOpen;
	if (str == L"ComboWindowClose")	return EGameEvent::ComboWindowClose;
	if (str == L"InvincibleStart")	return EGameEvent::InvincibleStart;
	if (str == L"InvincibleEnd")	return EGameEvent::InvincibleEnd;
	if (str == L"Landed")			return EGameEvent::Landed;
	if (str == L"ParryWindowOpen")  return EGameEvent::ParryWindowOpen;
	if (str == L"ParryWindowClose") return EGameEvent::ParryWindowClose;
	if (str == L"Recover")			return EGameEvent::Recover;
	if (str == L"DoAction")			return EGameEvent::DoAction;
	if (str == L"FootStep")			return EGameEvent::Footstep;
	if (str == L"Grab")				return EGameEvent::Grab;
	if (str == L"PlaySFX")				return EGameEvent::PlaySFX;
	// 정의되지 않은 이벤트 이름
	assert(false && "Unknown animation event name");
	return EGameEvent::None;
}

void AnimEventHelper::ConnectAbilitySystem(CAnimator* animator, AbilitySystem* abilitySystem)
{
	assert(animator && abilitySystem);

	animator->OnFrameEvent.Add(
		[abilitySystem](const wstring& eventName)
		{
			EGameEvent event = ToGameEvent(eventName);
			abilitySystem->TriggerEvent(event);
		}
	);
}
