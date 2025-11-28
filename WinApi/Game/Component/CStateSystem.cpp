#include "pch.h"
#include "CStateSystem.h"

CStateSystem::CStateSystem()
	: currentTags(Tag_None)
{
}

CStateSystem::~CStateSystem()
{
}

//========================================
// 태그 관리
//========================================

void CStateSystem::AddTag(StateTag tag)
{
	StateTag oldTags = currentTags;

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
	StateTag remaining = tag;
	while (remaining != Tag_None)
	{
		StateTag singleBit = static_cast<StateTag>(remaining & -static_cast<int>(remaining));
		bitCountMap[singleBit]++;
		remaining = static_cast<StateTag>(remaining & ~singleBit);
	}
	currentTags = currentTags | tag;

	// 변경 이벤트
	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

void CStateSystem::AddTagUnique(StateTag tag)
{
	if (HasTag(tag))
		return;
	
	AddTag(tag);
}

void CStateSystem::RemoveTag(StateTag tag)
{
	StateTag oldTags = currentTags;

	// 복합 비트를 개별 비트로 분리하여 각각 카운트 감소
	StateTag remaining = tag;
	while (remaining != Tag_None)
	{
		StateTag singleBit = static_cast<StateTag>(remaining & -static_cast<int>(remaining));
		if (bitCountMap[singleBit] > 0)
		{
			bitCountMap[singleBit]--;
			if (bitCountMap[singleBit] == 0)
			{
				currentTags = currentTags & ~singleBit;
			}
		}
		remaining = static_cast<StateTag>(remaining & ~singleBit);
	}

	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

void CStateSystem::ClearTags()
{
	StateTag oldTags = currentTags;
	currentTags = Tag_None;
	bitCountMap.clear();

	if (oldTags != currentTags)
		OnStateChanged.Broadcast(oldTags, currentTags);
}

//========================================
// 태그 검사
//========================================

bool CStateSystem::HasTag(StateTag tag) const
{
	return (currentTags & tag) == tag;
}

bool CStateSystem::HasAllTags(StateTag tags) const
{
	return (currentTags & tags) == tags;
}

bool CStateSystem::HasAnyTag(StateTag tags) const
{
	return (currentTags & tags) != Tag_None;
}

//========================================
// 디버그
//========================================

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

	if (!result.empty())
		result.pop_back();  // 마지막 '|' 제거

	return result.empty() ? L"None" : result;
}

//========================================
// Component 인터페이스
//========================================

void CStateSystem::ComponentInit()
{
	currentTags = Tag_None;
}
