#include "pch.h"
#include "CStateSystem.h"

CStateSystem::CStateSystem()
	: currentTags(Tag_None)
{
}

CStateSystem::~CStateSystem()
{
}

// 태그 관리
void CStateSystem::AddTag(EStateTag tag)
{
	EStateTag oldTags = currentTags;

	// 상호 배타적 태그 처리: Grounded <-> Airborne
	if (tag & Tag_Grounded)
	{
		bitCountMap[Tag_Airborne] = 0;
		currentTags = currentTags & ~Tag_Airborne;
	}

	if (tag & Tag_Airborne)
	{
		bitCountMap[Tag_Grounded] = 0;
		currentTags = currentTags & ~Tag_Grounded;
	}

	// 복합 비트를 개별 비트로 분리하여 각각 카운팅
	EStateTag remaining = tag;
	while (remaining != Tag_None)
	{
		EStateTag singleBit = static_cast<EStateTag>(remaining & -static_cast<int>(remaining));
		bitCountMap[singleBit]++;
		remaining = static_cast<EStateTag>(remaining & ~singleBit);
	}
	currentTags = currentTags | tag;

	// 변경 이벤트
	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

void CStateSystem::AddTagUnique(EStateTag tag)
{
	if (HasTag(tag))
		return;
	
	AddTag(tag);
}

void CStateSystem::RemoveTag(EStateTag tag)
{
	EStateTag oldTags = currentTags;

	// 복합 비트를 개별 비트로 분리하여 각각 카운트 감소
	EStateTag remaining = tag;
	while (remaining != Tag_None)
	{
		EStateTag singleBit = static_cast<EStateTag>(remaining & -static_cast<int>(remaining));
		if (bitCountMap[singleBit] > 0)
		{
			bitCountMap[singleBit]--;
			if (bitCountMap[singleBit] == 0)
			{
				currentTags = currentTags & ~singleBit;
			}
		}
		remaining = static_cast<EStateTag>(remaining & ~singleBit);
	}

	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

void CStateSystem::RemoveTagAll(EStateTag tag)
{
	EStateTag oldTags = currentTags;

	// 복합 비트를 개별 비트로 분리하여 각각 카운트를 0으로 설정
	EStateTag remaining = tag;
	while (remaining != Tag_None)
	{
		EStateTag singleBit = static_cast<EStateTag>(remaining & -static_cast<int>(remaining));
		bitCountMap[singleBit] = 0;
		currentTags = currentTags & ~singleBit;
		remaining = static_cast<EStateTag>(remaining & ~singleBit);
	}

	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

void CStateSystem::ClearTags()
{
	EStateTag oldTags = currentTags;
	currentTags = Tag_None;
	bitCountMap.clear();

	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

// 태그 검사
bool CStateSystem::HasTag(EStateTag tag) const
{
	return (currentTags & tag) == tag;
}

bool CStateSystem::HasAllTags(EStateTag tags) const
{
	return (currentTags & tags) == tags;
}

bool CStateSystem::HasAnyTag(EStateTag tags) const
{
	return (currentTags & tags) != Tag_None;
}

// 디버그
wstring CStateSystem::GetStateTagString() const
{
	wstring result;

	if (currentTags & Tag_Grounded)			result += L"Grounded|";
	if (currentTags & Tag_Airborne)			result += L"Airborne|";
	if (currentTags & Tag_Attacking)		result += L"Attacking|";
	if (currentTags & Tag_Moving)			result += L"Moving|";
	if (currentTags & Tag_Invincible)		result += L"Invincible|";
	if (currentTags & Tag_Stunned)			result += L"Stunned|";
	if (currentTags & Tag_CanCombo)			result += L"CanCombo|";
	if (currentTags & Tag_Hit)				result += L"Hit|";
	if (currentTags & Tag_Sliding)			result += L"Sliding|";
	if (currentTags & Tag_Parrying)			result += L"Parrying|";
	if (currentTags & Tag_Jumping)			result += L"Jumping|";
	if (currentTags & Tag_AbilityAnimation)	result += L"AbilityPlaying|";
	if (currentTags & Tag_BlockMovement)	result += L"BlockMovement|";
	if (currentTags & Tag_StopVelocity)		result += L"StopVelocity|";
	if (currentTags & Tag_Crouching)		result += L"Crouching|";
	if (currentTags & Tag_AirAttackExhausted)	result += L"AirAttackExhausted|";
	if (currentTags & Tag_FlaskRemaining)		result += L"FlaskRemaining|";
	

	if (!result.empty())
		result.pop_back();  // 마지막 '|' 제거

	return result.empty() ? L"None" : result;
}

// Component 인터페이스
void CStateSystem::ComponentInit()
{
	currentTags = Tag_None;
}
