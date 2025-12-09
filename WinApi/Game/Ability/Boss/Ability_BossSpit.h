#pragma once
#include "Game/Ability/Ability.h"

class Ability_BossSpit : public Ability
{
public:
	/*~ Ability Interface ~*/
	EStateTag GetRequiredTags() const override { return Tag_Grounded; }
	EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit; }
	EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }
	float GetCooldown() const override { return 5.0f; }

	void OnActivate() override;
	void OnEnd() override;

private:
	void OnSpitStart();
	void OnSpitLoop();
	void OnSpitFinish();
	void SpawnProjectile();

	int currentSpitCount = 0;
	int maxSpitCount = 3;

	static constexpr float PROJECTILE_SPEED = 500.f;
	static constexpr float PROJECTILE_ANGLE = 30.f;  // degrees
};
