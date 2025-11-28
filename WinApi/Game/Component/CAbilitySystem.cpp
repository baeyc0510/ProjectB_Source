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

void CAbilitySystem::AddAbility(EAbility abilityType, Ability* ability)
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

	if (!CanActivateAbility(ability))
		return false;

	ActivateAbility(ability);
	return true;
}

bool CAbilitySystem::CanActivateAbility(Ability* ability) const
{
	if (!ability)
		return false;

	if (ability->IsActive())
		return false;

	if (ability->IsOnCooldown())
		return false;

	if (stateSystem)
	{
		StateTag required = ability->GetRequiredTags();
		StateTag blocked = ability->GetBlockedTags();

		if (required != Tag_None && !stateSystem->HasAllTags(required))
			return false;

		if (blocked != Tag_None && stateSystem->HasAnyTag(blocked))
			return false;
	}

	return true;
}

void CAbilitySystem::ActivateAbility(Ability* ability)
{
	// CancelTags에 해당하는 Ability들 취소
	StateTag cancelTags = ability->GetCancelTags();
	if (cancelTags != Tag_None)
		CancelAbilitiesWithTag(cancelTags);

	// 태그 조작
	if (stateSystem)
	{
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
}

void CAbilitySystem::CancelAbility(EAbility abilityType)
{
	Ability* ability = GetAbility(abilityType);
	if (ability && ability->IsActive())
		ability->CancelAbility();
}

void CAbilitySystem::CancelAllAbilities()
{
	// 복사본으로 순회 (Cancel 중 activeAbilities 변경 가능)
	vector<Ability*> abilitiesToCancel = activeAbilities;
	for (Ability* ability : abilitiesToCancel)
	{
		if (ability->IsActive())
			ability->CancelAbility();
	}
}

void CAbilitySystem::CancelAbilitiesWithTag(StateTag tag)
{
	vector<Ability*> abilitiesToCancel;

	for (Ability* ability : activeAbilities)
	{
		if (ability->IsActive())
		{
			StateTag abilityTags = ability->GetTagsToAdd();
			if ((abilityTags & tag) != Tag_None)
				abilitiesToCancel.push_back(ability);
		}
	}

	for (Ability* ability : abilitiesToCancel)
	{
		ability->CancelAbility();
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
// Component 업데이트
//========================================

void CAbilitySystem::ComponentUpdate()
{
	// 모든 Ability 쿨다운 업데이트
	float deltaTime = DT;
	for (auto& pair : abilities)
	{
		pair.second->UpdateCooldown(deltaTime);
	}
}

//========================================
// 내부 함수
//========================================

void CAbilitySystem::OnAbilityEnded(Ability* ability)
{
	// 태그 자동 제거
	if (stateSystem)
	{
		StateTag tagsToRemove = ability->GetTagsToAdd();
		if (tagsToRemove != Tag_None)
			stateSystem->RemoveTag(tagsToRemove);
	}

	// activeAbilities에서 제거
	auto iter = find(activeAbilities.begin(), activeAbilities.end(), ability);
	if (iter != activeAbilities.end())
		activeAbilities.erase(iter);
}
