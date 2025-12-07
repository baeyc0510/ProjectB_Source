#pragma once
#include "Game/Enum.h"
#include "Game/Component/CAbilitySystem.h"

// WaitEvent용 - 인자 없는 멤버 함수를 이벤트 콜백으로 바인딩 (source 무시)
#define BIND_EVENT(Object, Function) \
	[Object](CGameObject*) { (Object)->Function(); }

class CGameObject;
class CAnimator;
class CRigidbody;
class CBoxCollider;
class CStateSystem;

class Ability
{
public:
	Ability();
	virtual ~Ability();

	/*~ Ability Interface ~*/
	void Init(CGameObject* inOwner, CAbilitySystem* inAbilitySystem);

	// 발동 조건
	virtual EStateTag GetRequiredTags() const { return Tag_None; }
	virtual EStateTag GetBlockedTags() const { return Tag_None; }
	virtual EStateTag GetTagsToAdd() const { return Tag_None; }
	virtual EStateTag GetTagsToRemove() const { return Tag_None; }
	virtual EStateTag GetCancelTags() const { return Tag_None; }

	// 쿨다운
	virtual float GetCooldown() const { return 0.f; }
	bool IsOnCooldown() const { return cooldownRemaining > 0.f; }
	void UpdateCooldown(float deltaTime);

	// 생명주기
	void Activate();
	virtual void OnActivate();
	void EndAbility();
	virtual void OnEnd();
	virtual void CancelAbility();

	// 이벤트 대기
	DelegateHandle WaitEvent(EGameEvent eventType, function<void(CGameObject*)> callback);
	void EndWaitEvent(DelegateHandle& handle);
	void ClearEventHandles();

	// 상태 조회
	bool IsActive() const { return isActive; }
	CGameObject* GetOwner() const { return owner; }
	CAbilitySystem* GetAbilitySystem() const { return abilitySystem; }

	// 종료 이벤트
	Delegate<> OnEnded;

protected:
	// 컴포넌트 접근 (지연 캐싱)
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

	mutable CAnimator* cachedAnimator = nullptr;
	mutable CRigidbody* cachedRigidbody = nullptr;
	mutable CBoxCollider* cachedCollider = nullptr;
	mutable CStateSystem* cachedStateSystem = nullptr;
};
