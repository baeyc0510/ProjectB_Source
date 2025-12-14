#pragma once
#include "Game/Enum.h"
#include "Game/Component/AbilitySystem.h"

// 인자 없는 멤버 함수를 이벤트 콜백으로 바인딩 (source 무시)
#define BIND_EVENT(Object, Function) \
	[Object](GameObject*) { (Object)->Function(); }

class GameObject;
class Animator;
class Rigidbody;
class BoxCollider;
class StateSystem;
class StatComponent;
struct FShakeParams;

class Ability
{
public:
	Ability();
	virtual ~Ability();

	/*~ Ability Interface ~*/
	void Init(GameObject* inOwner, AbilitySystem* inAbilitySystem);

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
	DelegateHandle WaitEvent(EGameEvent eventType, function<void(GameObject*)> callback);
	void EndWaitEvent(DelegateHandle& handle);
	void ClearEventHandles();

	// 상태 조회
	bool IsActive() const { return isActive; }
	GameObject* GetOwner() const { return owner; }
	AbilitySystem* GetAbilitySystem() const { return abilitySystem; }

	// 종료 이벤트
	Delegate<> OnEnded;

protected:
	// 컴포넌트 접근 (지연 캐싱)
	Animator* GetAnimator() const;
	Rigidbody* GetRigidbody() const;
	BoxCollider* GetCollider() const;
	StateSystem* GetStateSystem() const;
	StatComponent* GetStatComponent() const;

	// 이벤트 브로드캐스트 헬퍼
	void PlaySFX(const wstring& key);
	void PlayBGM(const wstring& key, float volume = 1.f);
	void StopBGM();
	void SpawnVFX(const wstring& key, Vec2 pos, int direction);
	void ShakeCamera(const FShakeParams& params);
	void SetTimeScale(float scale, float duration);

protected:
	GameObject* owner;
	AbilitySystem* abilitySystem;
	bool isActive;
	float cooldownRemaining;
	vector<DelegateHandle> eventHandles;

	mutable Animator* cachedAnimator = nullptr;
	mutable Rigidbody* cachedRigidbody = nullptr;
	mutable BoxCollider* cachedCollider = nullptr;
	mutable StateSystem* cachedStateSystem = nullptr;
	mutable StatComponent* cachedStatComponent = nullptr;
};
