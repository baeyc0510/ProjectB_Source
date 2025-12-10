#pragma once
#include "Character.h"
#include "Game/Interface/CombatInterface.h"

class AIController;

class Enemy : public Character, public ICombatInterface
{
public:
	Enemy();
	~Enemy() override;

	/*~ CEnemy Interface ~*/
	AIController* GetAIController() const { return aiController; }
	Vec2 GetParryPushbackForce() const { return parryPushbackForce; }
	
protected:
	/*~ CGameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	/*~ CCharacter Interface ~*/
	void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
	void OnDieComplete() override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

private:
	void UpdateAnimation();
	void UpdateAIMovement();
	void HandleMovementEvents();

	// UpdateAIMovement 헬퍼
	void UpdatePatrolMovement();
	void UpdateChaseMovement();
	void MoveInDirection(int dir, float speed);

protected:
	Vec2 parryPushbackForce = Vec2(200.f, 0.f);
	AIController* aiController = nullptr;
};
