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
	
	// Ability 생명주기
	virtual void Activate();
	virtual void End();
	virtual void Cancel();
	
	// 이벤트 시스템
	void WaitEvent(EGameEvent eventType, function<void()> callback);
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
	vector<pair<EGameEvent, DelegateHandle>> eventHandles;
};
