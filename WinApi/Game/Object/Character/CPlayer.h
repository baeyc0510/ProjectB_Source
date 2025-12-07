#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"
#include "Game/Interface/CombatInterface.h"

class CPlayer : public CCharacter, public ICombatInterface
{
public:
	CPlayer();
	~CPlayer() override;

	/*~ Player Interface ~*/
	void SetCurrentHP(float value);
	void SetMaxHP(float value);
	void SetCurrentMP(float value);
	void SetMaxMP(float value);
	void SetCurrentFlask(int value);
	void SetMaxFlask(int value);
	void SetJumpForce(float value) { jumpForce = value; }
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
	
	float GetCurrentHP() const {return currentHP;}
	float GetMaxHP() const {return maxHP;}
	float GetCurrentMP() const {return currentMP;}
	float GetMaxMP() const {return maxMP;}
	int GetCurrentFlask() const {return currentFlask;}
	int GetMaxFlask() const {return maxFlask;}
	float GetJumpForce() const {return jumpForce;}
	
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
	void UpdateHP(float& attribute, float value) const;
	void UpdateMP(float& attribute, float value) const;
	
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
	void TryAutoLedgeClimb();

	// Post Update
	void PostUpdate();
	void CheckVelocityChanged();

	// Animation
	void UpdateAnimation();

	// Events
	void OnFootstep();

	void CheckLedge(CCollider* other);
	void ClearLedge();

private:
	static constexpr float MOVE_SPEED = 250.f;
	static constexpr float JUMP_FORCE = 490.f;
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float MAX_HP = 100.f;
	static constexpr float MAX_MP = 100.f;
	static constexpr int MAX_FLASK = 2;
	static constexpr float LEDGE_CLIMB_THRESHOLD = 10.f;

	Vec2 characterScale;
	Vec2 colOffset;

	float currentHP;
	float maxHP;
	float currentMP;
	float maxMP;
	int currentFlask;
	int maxFlask;
	float jumpForce;

	Vec2 prevVelocity;

	float ladderX;
	float ladderTopY;
	float ladderBottomY;

	bool bOverlapWithLedge = false;
	UINT ledgeId = 0;
	float ledgeX = -FLT_MAX;
	float ledgeTop = -FLT_MAX;
	int ledgeDirection = 0;

	bool bWasMovingInput;
};