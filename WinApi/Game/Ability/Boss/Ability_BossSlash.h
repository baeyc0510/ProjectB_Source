#pragma once
#include "Game/Ability/Ability.h"

class Ability_BossSlash : public Ability
{
public:
	/*~ Ability Interface ~*/
	EStateTag GetRequiredTags() const override { return Tag_Grounded; }
	EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
	EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }
	float GetCooldown() const override { return 2.0f; }

	void OnActivate() override;
	void OnEnd() override;

private:
	void OnHitCheck();

	Vec2 GetTraceOffset() const;
	Vec2 GetTraceSize() const;

	static constexpr float DAMAGE = 20.f;
};
