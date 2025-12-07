#pragma once
#include "Game/Enum.h"

// WaitEvent용 - 인자 없는 멤버 함수를 이벤트 콜백으로 바인딩 (source 무시)
#define BIND_EVENT(Object, Function) \
	[Object](CGameObject*) { (Object)->Function(); }

class CGameObject;
class CAbilitySystem;
class CAnimator;
class CRigidbody;
class CBoxCollider;
class CStateSystem;

class Ability
{
public:
	Ability();
	virtual ~Ability();

	// AbilitySystem에서 초기화
	void Init(CGameObject* inOwner, CAbilitySystem* inAbilitySystem);

	// Ability 조건
	virtual EStateTag GetRequiredTags() const { return Tag_None; }
	virtual EStateTag GetBlockedTags() const { return Tag_None; }
	virtual EStateTag GetTagsToAdd() const { return Tag_None; }
	virtual EStateTag GetTagsToRemove() const { return Tag_None; }

	// Cancel 시 다른 Ability 취소 (이 태그를 가진 Ability들을 취소)
	virtual EStateTag GetCancelTags() const { return Tag_None; }

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
	// 컴포넌트 헬퍼 (지연 초기화)
	CAnimator* GetAnimator() const;
	CRigidbody* GetRigidbody() const;
	CBoxCollider* GetCollider() const;
	CStateSystem* GetStateSystem() const;

protected:
	CGameObject* owner;
	CAbilitySystem* abilitySystem;
	bool isActive;
	float cooldownRemaining;
	vector<DelegateHandle> eventHandles;

	// 캐싱된 컴포넌트 (mutable: const 메서드에서 캐싱 가능)
	mutable CAnimator* cachedAnimator = nullptr;
	mutable CRigidbody* cachedRigidbody = nullptr;
	mutable CBoxCollider* cachedCollider = nullptr;
	mutable CStateSystem* cachedStateSystem = nullptr;
};
