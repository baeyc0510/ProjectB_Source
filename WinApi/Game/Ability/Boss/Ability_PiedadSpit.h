#pragma once
#include "Game/Ability/Ability.h"

class Ability_PiedadSpit : public Ability
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
	float CalculateSpeedForDistance(float targetDistance) const;

private:
	static constexpr float PROJECTILE_ANGLE = 30.f;  // degrees
	static constexpr float DISTANCE_OFFSET = 30.f;   // SpitMinRange 전후 간격

	int currentSpitCount = 0;
	int maxSpitCount = 3;
};
