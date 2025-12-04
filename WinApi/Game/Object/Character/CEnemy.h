#pragma once
#include "CCharacter.h"
#include "Game/Interface/CombatInterface.h"

class CAIController;

class CEnemy : public CCharacter, public ICombatInterface
{
public:
	CEnemy();
	~CEnemy() override;

	CAIController* GetAIController() const { return aiController; }

protected:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	void OnCollisionStay(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;

	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

private:
	void UpdateAnimation();
	void UpdateAIMovement();
	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);

	static constexpr float KNOCKBACK_POWER = 1.0f;
	static constexpr float GROUND_CHECK_AHEAD = 5.0f;	// 앞쪽 체크 거리
	static constexpr float GROUND_CHECK_BELOW = 5.0f;	// 아래쪽 체크 거리
	static constexpr float GROUND_CHECK_SIZE = 5.0f;	// 체크 박스 크기

	CAIController* aiController = nullptr;
};

