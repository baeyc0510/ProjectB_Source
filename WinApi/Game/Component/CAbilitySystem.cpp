#include "pch.h"
#include "CAbilitySystem.h"
#include "CStateSystem.h"
#include "Game/Ability/Ability.h"

CAbilitySystem::CAbilitySystem()
	: stateSystem(nullptr)
{
}

CAbilitySystem::~CAbilitySystem()
{
}

//========================================
// Ability 관리
//========================================

void CAbilitySystem::RegisterAbility(EAbility abilityType, Ability* ability)
{
	abilities[abilityType] = unique_ptr<Ability>(ability);
	ability->Init(GetOwner(), this);
}

Ability* CAbilitySystem::GetAbility(EAbility abilityType) const
{
	auto iter = abilities.find(abilityType);
	if (iter != abilities.end())
		return iter->second.get();
	return nullptr;
}

bool CAbilitySystem::TryActivateAbility(EAbility abilityType)
{
	Ability* ability = GetAbility(abilityType);
	if (!ability)
		return false;

	// 이미 활성화된 경우
	if (ability->IsActive())
		return false;

	// State 조건 체크
	if (stateSystem)
	{
		StateTag required = ability->GetRequiredTags();
		StateTag blocked = ability->GetBlockedTags();

		// 필요 태그 확인
		if (required != Tag_None && !stateSystem->HasAllTags(required))
			return false;

		// 금지 태그 확인
		if (blocked != Tag_None && stateSystem->HasAnyTag(blocked))
			return false;

		// 태그 조작
		StateTag toRemove = ability->GetTagsToRemove();
		StateTag toAdd = ability->GetTagsToAdd();

		if (toRemove != Tag_None)
			stateSystem->RemoveTag(toRemove);

		if (toAdd != Tag_None)
			stateSystem->AddTag(toAdd);
	}

	// Ability 활성화
	ability->Activate();
	activeAbilities.push_back(ability);

	// 종료 이벤트 구독
	ability->OnEnded.Bind([this, ability]() { OnAbilityEnded(ability); });

	return true;
}

void CAbilitySystem::CancelAbility(EAbility abilityType)
{
	Ability* ability = GetAbility(abilityType);
	if (ability && ability->IsActive())
		ability->Cancel();
}

void CAbilitySystem::CancelAllAbilities()
{
	for (Ability* ability : activeAbilities)
	{
		if (ability->IsActive())
			ability->Cancel();
	}
}

//========================================
// 이벤트 중계
//========================================

void CAbilitySystem::TriggerEvent(EGameEvent eventType)
{
	OnEvent.Broadcast(eventType);
}

//========================================
// 상태 조회
//========================================

bool CAbilitySystem::IsAbilityActive(EAbility abilityType) const
{
	Ability* ability = GetAbility(abilityType);
	return ability && ability->IsActive();
}

//========================================
// Component 인터페이스
//========================================

void CAbilitySystem::ComponentInit()
{
	// StateSystem 캐싱
	stateSystem = GetOwner()->GetComponent<CStateSystem>();
}

void CAbilitySystem::ComponentRelease()
{
	abilities.clear();			// unique_ptr이 자동으로 delete
	activeAbilities.clear();
}

//========================================
// 내부 함수
//========================================

void CAbilitySystem::OnAbilityEnded(Ability* ability)
{
	auto iter = find(activeAbilities.begin(), activeAbilities.end(), ability);
	if (iter != activeAbilities.end())
		activeAbilities.erase(iter);
}
