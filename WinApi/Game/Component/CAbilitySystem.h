#pragma once
#include "Game/Enum.h"

class Ability;
class CStateSystem;

class CAbilitySystem : public Component<CGameObject>
{
public:
	CAbilitySystem();
	virtual ~CAbilitySystem();
	
	void AddAbility(EAbility abilityType, Ability* ability);
	Ability* GetAbility(EAbility abilityType) const;

	bool TryActivateAbility(EAbility abilityType);
	bool CanActivateAbility(Ability* ability) const;
	void ActivateAbility(Ability* ability);

	void CancelAbility(EAbility abilityType);
	void CancelAllAbilities();
	void CancelAbilitiesWithTag(StateTag tag);
	
	// 이벤트 중계
	void TriggerEvent(EGameEvent eventType);
	
	// 상태 조회
	bool IsAbilityActive(EAbility abilityType) const;

	// 어빌리티에서 캐시된 StateSystem에 접근하기 위한 함수
	CStateSystem* GetStateSystem() const { return stateSystem; }

protected:
	void ComponentInit() override;
	void ComponentOnEnable() override {}
	void ComponentUpdate() override;
	void ComponentRender() override {}
	void ComponentOnDisable() override {}
	void ComponentRelease() override;
	
private:
	void OnAbilityEnded(Ability* ability);

public:
	// AbilitySystem 이벤트
	MulticastDelegate<EGameEvent> OnEvent;
	
private:
	map<EAbility, unique_ptr<Ability>>	abilities;
	vector<Ability*> activeAbilities;
	CStateSystem* stateSystem;
};
