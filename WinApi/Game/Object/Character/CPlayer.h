#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"

class CPlayer : public CCharacter, public ICombatInterface
{
public:
	CPlayer();
	~CPlayer() override;

protected:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

	/*~ Player Interface ~*/
	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);
	wstring GetPlayerHitVfxKey(EDamageType damageType);

	void SetCurrentHP(float value);
	void SetMaxHP(float value);
	void SetCurrentMP(float value);
	void SetMaxMP(float value);
	void UpdateHP(float& attribute, float value) const;
	void UpdateMP(float& attribute, float value) const;
	
	float GetCurrentHP() const {return currentHP;}
	float GetMaxHP() const {return maxHP;}
	float GetCurrentMP() const {return currentMP;}
	float GetMaxMP() const {return maxMP;}
	
private:
	// 입력 처리
	void HandleCombatInput();
	void UpdateMovement();
	void HandleActionInput();

	// 상태 처리
	void UpdateAnimation();
	void OnStateChanged(StateTag oldTags, StateTag newTags);

private:
	// 상수
	static constexpr float MOVE_SPEED = 300.f;
	static constexpr float JUMP_FORCE = 600.f;
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float MAX_HP = 100.f;
	static constexpr float MAX_MP = 100.f;

	// Crouch collider 설정
	Vec2 standingColScale;
	Vec2 standingColOffset;
	
	// status
	float currentHP;
	float maxHP;
	
	float currentMP;
	float maxMP;
};