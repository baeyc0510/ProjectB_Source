#pragma once
#include "Character.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Util/LedgeHelper.h"

class Player : public Character, public ICombatInterface
{
public:
	Player();
	~Player() override;

	/*~ Character Interface ~*/
	Vec2 GetPushbackForce(EDamageType damageType) const override;
	
	/*~ Player Interface ~*/
	Vec2 GetCenterPos();
	
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

public:
	// 피격 이벤트 델리게이트 (Parry 등에서 사용)
	MulticastDelegate<GameObject*, const CombatContext&> OnDamageReceived;

protected:
	/*~ GameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void LateUpdate() override;
	void OnCollisionEnter(Collider* other) override;
	void OnCollisionStay(Collider* other) override;
	void OnCollisionExit(Collider* other) override;
	
	/*~ Character Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	void HandleAnimationEvent(EGameEvent event) override;
	bool ShouldIgnorePlatform() const override;
	void OnDieStart() override;
	void OnDieComplete() override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(GameObject* source, const CombatContext& context) override;
	bool IsDead() override;
	
	/*~ Player Interface ~*/
	void InitStartupStats();
	void OnStatChanged(EStatType type, float& current, float& max) override;

	wstring GetPlayerHitVfxKey(EDamageType damageType);

	// OnDamage 헬퍼
	void SpawnPlayerDamageVFX(const CombatContext& context, int spawnDirection);
	void ApplyHitReaction(float dir, Vec2 force, EDamageType damageType);
	bool ProcessGuardInteraction(EDamageType damageType, float dir);
	
private:
	// Active Input
	void HandleReturnToTitleInput();
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
	void CheckVelocitySignChanged();

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
	static constexpr float PUSHBACK_FORCE_X = 250.f;
	static constexpr float PUSHBACK_FORCE_Y = 150.f;
	static constexpr float SUPER_HEAVY_PUSHBACK_MULT = 1.6f;
	static constexpr float HEAVY_GUARD_PUSHBACK_MULT = 2.0f;

	// 캐릭터 크기
	static constexpr float CHARACTER_WIDTH = 33.f;
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
	LedgeHelper ledgeHelper;

	bool bWasMovingInput = false;
	bool bIsDown = false;
	
	// 사망 연출용 타이머
	SafeTimerHandle deathTimerHandle;
};