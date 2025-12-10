#include "pch.h"
#include "AbilitySystem.h"
#include "StateSystem.h"
#include "Game/Ability/Ability.h"

AbilitySystem::AbilitySystem()
	: stateSystem(nullptr)
{
}

AbilitySystem::~AbilitySystem()
{
}

// Ability 관리
void AbilitySystem::AddAbility(EAbility abilityType, Ability* ability)
{
	abilities[abilityType] = unique_ptr<Ability>(ability);
	ability->Init(GetOwner(), this);
}

Ability* AbilitySystem::GetAbility(EAbility abilityType) const
{
	auto iter = abilities.find(abilityType);
	if (iter != abilities.end())
		return iter->second.get();
	return nullptr;
}

bool AbilitySystem::TryActivateAbility(EAbility abilityType)
{
	Ability* ability = GetAbility(abilityType);
	if (!ability)
		return false;

	if (!CanActivateAbility(ability))
		return false;

	ActivateAbility(ability);
	return true;
}

bool AbilitySystem::CanActivateAbility(Ability* ability) const
{
	if (!ability)
		return false;

	if (ability->IsActive())
		return false;

	if (ability->IsOnCooldown())
		return false;

	if (stateSystem)
	{
		EStateTag required = ability->GetRequiredTags();
		EStateTag blocked = ability->GetBlockedTags();

		if (required != Tag_None && !stateSystem->HasAllTags(required))
			return false;

		if (blocked != Tag_None && stateSystem->HasAnyTag(blocked))
			return false;
	}

	return true;
}

bool AbilitySystem::CanActivateAbility(EAbility abilityType) const
{
	Ability* ability = GetAbility(abilityType);
	return CanActivateAbility(ability);
}

void AbilitySystem::ActivateAbility(Ability* ability)
{
	// CancelTags에 해당하는 Ability들 취소
	EStateTag cancelTags = ability->GetCancelTags();
	if (cancelTags != Tag_None)
		CancelAbilitiesWithTag(cancelTags);

	// 태그 조작
	if (stateSystem)
	{
		EStateTag toRemove = ability->GetTagsToRemove();
		EStateTag toAdd = ability->GetTagsToAdd();

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

void AbilitySystem::CancelAbility(EAbility abilityType)
{
	Ability* ability = GetAbility(abilityType);
	if (ability && ability->IsActive())
		ability->CancelAbility();
}

void AbilitySystem::CancelAllAbilities()
{
	// 복사본으로 순회 (Cancel 중 activeAbilities 변경 가능)
	vector<Ability*> abilitiesToCancel = activeAbilities;
	for (Ability* ability : abilitiesToCancel)
	{
		if (ability->IsActive())
			ability->CancelAbility();
	}
}

void AbilitySystem::CancelAbilitiesWithTag(EStateTag tag)
{
	vector<Ability*> abilitiesToCancel;

	for (Ability* ability : activeAbilities)
	{
		if (ability->IsActive())
		{
			EStateTag abilityTags = ability->GetTagsToAdd();
			if ((abilityTags & tag) != Tag_None)
				abilitiesToCancel.push_back(ability);
		}
	}

	for (Ability* ability : abilitiesToCancel)
	{
		ability->CancelAbility();
	}
}

// 이벤트 중계
void AbilitySystem::TriggerEvent(EGameEvent eventType, CGameObject* source)
{
	OnEvent.Broadcast(eventType, source);
}

// 상태 조회
bool AbilitySystem::IsAbilityActive(EAbility abilityType) const
{
	Ability* ability = GetAbility(abilityType);
	return ability && ability->IsActive();
}

// Component 인터페이스
void AbilitySystem::ComponentInit()
{
	// StateSystem 캐싱
	stateSystem = GetOwner()->GetComponent<StateSystem>();
}

void AbilitySystem::ComponentRelease()
{
	abilities.clear();			// unique_ptr이 자동으로 delete
	activeAbilities.clear();
}

// Component 업데이트
void AbilitySystem::ComponentUpdate()
{
	// 모든 Ability 쿨다운 업데이트
	float deltaTime = DT;
	for (auto& pair : abilities)
	{
		pair.second->UpdateCooldown(deltaTime);
	}
}

// 내부 함수
void AbilitySystem::OnAbilityEnded(Ability* ability)
{
	// 태그 자동 제거
	if (stateSystem)
	{
		EStateTag tagsToRemove = ability->GetTagsToAdd();
		if (tagsToRemove != Tag_None)
			stateSystem->RemoveTag(tagsToRemove);
	}

	// activeAbilities에서 제거
	auto iter = find(activeAbilities.begin(), activeAbilities.end(), ability);
	if (iter != activeAbilities.end())
		activeAbilities.erase(iter);
}
