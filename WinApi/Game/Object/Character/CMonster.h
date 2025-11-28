#pragma once
#include "CCharacter.h"
#include "Game/Interface/CombatInterface.h"

class CMonster : public CCharacter, public ICombatInterface
{
public:
	CMonster();
	~CMonster() override;

protected:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

private:
	void UpdateAnimation();
	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);

	// 상수
	static constexpr float ATTACK_INTERVAL = 1.5f;
	static constexpr float KNOCKBACK_POWER = 1.0f;

	// 런타임 변수
	float attackTimer = 0.0f;
};

