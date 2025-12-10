#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Util/CLedgeHelper.h"

class CPlayer : public CCharacter, public ICombatInterface
{
public:
	CPlayer();
	~CPlayer() override;

	/*~ Player Interface ~*/
	void SetLadderInfo(float x, float topY, float bottomY) { ladderX = x; ladderTopY = topY; ladderBottomY = bottomY; }

	float GetLadderX() const { return ladderX; }
	float GetLadderTopY() const { return ladderTopY; }
	float GetLadderBottomY() const { return ladderBottomY; }
	Vec2 GetCharacterScale() const { return characterScale; }

	// Ledge 정보
	float GetLedgeX() const { return ledgeHelper.GetLedgeX(); }
	float GetLedgeTop() const { return ledgeHelper.GetLedgeTop(); }
	int GetLedgeDirection() const { return ledgeHelper.GetLedgeDirection(); }
	void ResetLedgeInfo() { ledgeHelper.ClearLedge(); }

	
	// Down 연출
	void SetIsDown(bool value) { bIsDown = value; }
	
protected:
	/*~ CGameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void LateUpdate() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;
	
	/*~ CCharacter Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	void HandleAnimationEvent(EGameEvent event) override;
	bool ShouldIgnorePlatform() const override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

	/*~ Player Interface ~*/
	void InitStartupStats();
	void OnStatChanged(EStatType type, float current, float max) override;

	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);
	wstring GetPlayerHitVfxKey(EDamageType damageType);

	// OnDamage 헬퍼
	void SpawnPlayerDamageVFX(const CombatContext& context, int spawnDirection);
	void ApplyHitReaction(float dir, Vec2 force, EDamageType damageType);
	bool ProcessGuardInteraction(EDamageType damageType, float dir, Vec2& outForce);
	
private:
	// Active Input
	void ProcessActiveInput();
	void HandleCombatInput();
	void HandleActionInput();

	// State Updates
	void UpdatePlayerStates();
	void UpdateMovementState();
	void UpdateLedgeState();

	// Passive Abilities
	void ProcessPassiveAbilities();

	// Late Update
	void CheckVelocityChanged();

	// Animation
	void UpdateAnimation();

	// Events
	void OnFootstep();

public:
	static constexpr float PLAYER_GRAVITY_SCALE = 1.6f;

private:
	// 기본 스탯
	static constexpr float MOVE_SPEED = 250.f;
	static constexpr float MAX_HP = 300.f;
	static constexpr float MAX_MP = 100.f;
	static constexpr int MAX_FLASK = 2;
	static constexpr float JUMP_FORCE = 490.f;
	static constexpr float ATTACK_POWER = 100.f;

	// 넉백/피격
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float PUSHBACK_FORCE_X = 300.f;
	static constexpr float PUSHBACK_FORCE_Y = 150.f;
	static constexpr float SUPER_HEAVY_KNOCKBACK_MULT = 1.6f;
	static constexpr float HEAVY_GUARD_PUSHBACK_MULT = 2.0f;

	// 캐릭터 크기
	static constexpr float CHARACTER_WIDTH = 42.f;
	static constexpr float CHARACTER_HEIGHT = 66.f;
	static constexpr float COLLIDER_OFFSET_Y = -33.f;
	static constexpr float CROUCH_HEIGHT_SCALE = 0.5f;

	// 이동
	static constexpr float MAX_SLOPE_ANGLE = 50.0f;
	static constexpr float LEDGE_CLIMB_THRESHOLD = 10.f;

	Vec2 characterScale;
	Vec2 colOffset;
	Vec2 prevVelocity;

	// Ladder 정보
	float ladderX = 0;
	float ladderTopY = 0;
	float ladderBottomY = 0;

	// Ledge 헬퍼
	CLedgeHelper ledgeHelper;

	bool bWasMovingInput = false;
	bool bIsDown = false;
};