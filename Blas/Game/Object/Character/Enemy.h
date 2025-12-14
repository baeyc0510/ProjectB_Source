#pragma once
#include "Character.h"
#include "Game/Interface/CombatInterface.h"

class AIController;

class Enemy : public Character, public ICombatInterface
{
public:
	Enemy();
	~Enemy() override;

	/*~ Enemy Interface ~*/
	AIController* GetAIController() const { return aiController; }
	Vec2 GetParryPushbackForce() const { return parryPushbackForce; }
	
protected:
	/*~ GameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;

	/*~ Character Interface ~*/
	void OnDieComplete() override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(GameObject* source, const CombatContext& context) override;
	bool IsDead() override;
	
	/*~ Enemy Interface ~*/
	virtual void UpdateAnimation();
	virtual void UpdateAIActions();
	virtual void MoveInDirection(int dir, float speed);
	
private:
	void HandleMovementEvents();

	// UpdateAIMovement 헬퍼
	void UpdatePatrol();
	void UpdateChase();

protected:
	Vec2 parryPushbackForce = Vec2(200.f, 0.f);
	AIController* aiController = nullptr;
};
