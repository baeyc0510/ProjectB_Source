#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/CStatComponent.h"

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
	float GetLedgeX() const { return ledgeX; }
	float GetLedgeTop() const { return ledgeTop; }
	int GetLedgeDirection() const { return ledgeDirection; }
	void ResetLedgeInfo() { ClearLedge(); }

	
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
	void OnStatChanged(EStatType type, float current, float max);

	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);
	wstring GetPlayerHitVfxKey(EDamageType damageType);
	
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

	void CheckLedge(CCollider* other);
	void ClearLedge();

public:
	static constexpr float PLAYER_GRAVITY_SCALE = 1.6f;
	
private:
	static constexpr float MOVE_SPEED = 250.f;
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float MAX_HP = 300.f;
	static constexpr float MAX_MP = 100.f;
	static constexpr int MAX_FLASK = 2;
	static constexpr float LEDGE_CLIMB_THRESHOLD = 10.f;

	Vec2 characterScale;
	Vec2 colOffset;
	Vec2 prevVelocity;

	// Ladder 정보
	float ladderX = 0;
	float ladderTopY = 0;
	float ladderBottomY = 0;

	bool bOverlapWithLedge = false;
	UINT ledgeId = 0;
	float ledgeX = -FLT_MAX;
	float ledgeTop = -FLT_MAX;
	int ledgeDirection = 0;

	bool bWasMovingInput = false;
	bool bIsDown = false;
};