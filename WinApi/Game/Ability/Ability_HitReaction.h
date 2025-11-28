#pragma once
#include "Ability.h"

class Ability_HitReaction : public Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_None; }
    StateTag GetBlockedTags() const override { return Tag_Invincible | Tag_SpecialAction; }
    StateTag GetTagsToAdd() const override { return Tag_Hit | Tag_BlockMovement | Tag_AbilityAnimation; }
    StateTag GetCancelTags() const override { return Tag_Attacking; }

    float GetCooldown() const override {return 0.0f;} // TODO: 캐릭터마다 다르게 적용

    void OnActivate() override;
    void OnEnd() override;

private:
    void StopKnockback();
};
