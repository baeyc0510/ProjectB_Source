#pragma once
#include "CCharacter.h"
#include "Game/Interface/CombatInterface.h"

class CAIController;

class CEnemy : public CCharacter, public ICombatInterface
{
public:
	CEnemy();
	~CEnemy() override;

	/*~ CEnemy Interface ~*/
	CAIController* GetAIController() const { return aiController; }
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

protected:
	Vec2 parryPushbackForce = Vec2(200.f, 0.f);
	CAIController* aiController = nullptr;
};
