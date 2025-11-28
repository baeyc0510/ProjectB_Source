#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"

class CPlayer : public CCharacter, public ICombatInterface
{
public:
	CPlayer();
	~CPlayer() override;

	CCollider* GetCollider() { return collider; }

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

private:
	// 입력 처리
	void HandleCombatInput();
	void HandleMovementInput();
	void HandleActionInput();

	// 상태 처리
	void UpdateAnimation();
	void OnStateChanged(StateTag oldTags, StateTag newTags);

private:
	// 상수
	static constexpr float MOVE_SPEED = 300.f;
	static constexpr float JUMP_FORCE = 500.f;
	static constexpr float KNOCKBACK_POWER = 100.f;

	// Crouch collider 설정
	Vec2 standingColScale;
	Vec2 standingColOffset;
};