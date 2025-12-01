#pragma once
#include "Game/Enum.h"

class CGameObject;
class CAbilitySystem;

class Ability
{
public:
	Ability();
	virtual ~Ability();

	// AbilitySystem에서 초기화
	void Init(CGameObject* inOwner, CAbilitySystem* inAbilitySystem);

	// Ability 조건
	virtual StateTag GetRequiredTags() const { return Tag_None; }
	virtual StateTag GetBlockedTags() const { return Tag_None; }
	virtual StateTag GetTagsToAdd() const { return Tag_None; }
	virtual StateTag GetTagsToRemove() const { return Tag_None; }

	// Cancel 시 다른 Ability 취소 (이 태그를 가진 Ability들을 취소)
	virtual StateTag GetCancelTags() const { return Tag_None; }

	// 쿨다운
	virtual float GetCooldown() const { return 0.f; }
	bool IsOnCooldown() const { return cooldownRemaining > 0.f; }
	void UpdateCooldown(float deltaTime);

	// Ability 생명주기
	void Activate();
	virtual void OnActivate();
	void EndAbility();
	virtual void OnEnd();
	virtual void CancelAbility();

	// 이벤트 시스템
	DelegateHandle WaitEvent(EGameEvent eventType, function<void(CGameObject*)> callback);
	void EndWaitEvent(DelegateHandle& handle);
	void ClearEventHandles();

	bool IsActive() const { return isActive; }
	CGameObject* GetOwner() const { return owner; }
	CAbilitySystem* GetAbilitySystem() const { return abilitySystem; }

public:
	// 종료 이벤트
	Delegate<> OnEnded;

protected:
	CGameObject* owner;
	CAbilitySystem* abilitySystem;
	bool isActive;
	float cooldownRemaining;
	vector<DelegateHandle> eventHandles;
};
