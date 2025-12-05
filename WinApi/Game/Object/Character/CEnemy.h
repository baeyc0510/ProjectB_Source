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

	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

private:
	void UpdateAnimation();
	void UpdateAIMovement();
	void HandleMovementEvents();
	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);

	static constexpr float KNOCKBACK_POWER = 1.0f;

	CAIController* aiController = nullptr;
};
