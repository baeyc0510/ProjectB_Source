#pragma once
#include "Game/Ability/Ability.h"

class Ability_BossGroundSmash : public Ability
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
	void PlaySmashSound();
	void OnSmashImpact();
	void OnSmashToIdle();
	void SpawnSpikes();
	int CalculateSpikeCount() const;

	static constexpr float DAMAGE = 25.f;
	static constexpr int MAX_SPIKES_PER_SIDE = 5;
	static constexpr float SPIKE_SPACING = 60.f;
	static constexpr float SPIKE_SPAWN_DELAY_INTERVAL = 0.1f;
};
