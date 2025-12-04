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
	
	float GetCurrentHP() const {return currentHP;}
	float GetMaxHP() const {return maxHP;}
	float GetCurrentMP() const {return currentMP;}
	float GetMaxMP() const {return maxMP;}
	int GetCurrentFlask() const {return currentFlask;}
	int GetMaxFlask() const {return maxFlask;}
	float GetJumpForce() const {return jumpForce;}
	
protected:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	/*~ CCharacter Interface ~*/
	void OnCollisionEnter(CCollider* other) override;
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;

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
	
private:
	// 상수
	static constexpr float MOVE_SPEED = 250.f;
	static constexpr float JUMP_FORCE = 470.f;
	static constexpr float KNOCKBACK_POWER = 100.f;
	static constexpr float MAX_HP = 100.f;
	static constexpr float MAX_MP = 100.f;
	static constexpr int MAX_FLASK = 2;
	static constexpr float CLIMB_SPEED = 100.f;

	// Crouch collider 설정
	Vec2 colScale;
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

	// 입력 상태 변화 감지용
	bool bWasMovingInput;
};