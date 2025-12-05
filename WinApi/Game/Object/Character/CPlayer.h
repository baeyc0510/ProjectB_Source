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
	
	void Render() override;
	void OnDisable() override;
	void Release() override;
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;
	
	/*~ CCharacter Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
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
	// 입력 처리
	void HandleCombatInput();
	void UpdateMovement();
	void HandleActionInput();

	// 상태 처리
	void UpdateAnimation();
	void CheckVelocityChanged();

	void CheckLedge(CCollider* other);
	void UpdateCanClimbLedge();
	void ClearLedge();
private:
	// 상수
	static constexpr float MOVE_SPEED = 250.f;
	static constexpr float JUMP_FORCE = 470.f;
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float MAX_HP = 100.f;
	static constexpr float MAX_MP = 100.f;
	static constexpr int MAX_FLASK = 2;
	static constexpr float LEDGE_CLIMB_THRESHOLD = 10.f;
	
	// Crouch collider 설정
	Vec2 characterScale;
	Vec2 colOffset;
	
	// status
	float currentHP;
	float maxHP;
	
	float currentMP;
	float maxMP;
	
	int currentFlask;
	int maxFlask;
	
	float jumpForce;

	// velocity 변경 감지용
	Vec2 prevVelocity;
	
	// 사다리 정보
	float ladderX;
	float ladderTopY;
	float ladderBottomY;
	
	// Ledge(땅 끄트머리 턱) 정보
	bool bOverlapWithLedge = false;
	UINT ledgeId = 0;
	float ledgeX = -FLT_MAX;
	float ledgeTop = -FLT_MAX;
	
	// 입력 상태 변화 감지용
	bool bWasMovingInput;
};