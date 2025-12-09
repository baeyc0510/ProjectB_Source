#pragma once
#include "Game/AnimKey.h"
#include "Game/Ability/Ability.h"

class Ability_HitReaction : public Ability
{
public:
    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_None; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Invincible | Tag_SpecialAction; }
    EStateTag GetTagsToAdd() const override { return Tag_Hit | Tag_BlockMovement | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Attacking; }
    float GetCooldown() const override { return 0.0f; }

    void OnActivate() override;
    void OnEnd() override;

protected:
    virtual wstring GetHitAnimKey() const;

private:
    void StopKnockback();
};

class Ability_ParryHitReaction : public Ability_HitReaction
{
protected:
    /*~ Ability_HitReaction Interface ~*/
    wstring GetHitAnimKey() const override { return AnimKey::ParryHit; }
};
