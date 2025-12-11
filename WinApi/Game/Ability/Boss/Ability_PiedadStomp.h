#pragma once
#include "Game/Ability/Ability.h"

class Ability_PiedadStomp : public Ability
{
public:
	/*~ Ability Interface ~*/
	EStateTag GetRequiredTags() const override { return Tag_Grounded; }
	EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit; }
	EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }
	float GetCooldown() const override { return 3.0f; }

	void OnActivate() override;
	void OnEnd() override;

private:
	void OnHitCheck();
	void SpawnHazard();
	void ScheduleSpikeSpawn(Vec2 landingPos);

	Vec2 GetTraceOffset() const;
	Vec2 GetTraceSize() const;

	static constexpr float DAMAGE = 15.f;
	static constexpr float LAUNCH_FORCE_X = 200.f;
	static constexpr float LAUNCH_FORCE_Y = -400.f;
};
