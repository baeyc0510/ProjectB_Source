#pragma once
#include "Game/Ability/Ability.h"

class Ability_Slide : public Ability
{
public:
    Ability_Slide();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit | Tag_BlockMovement; }
    EStateTag GetTagsToAdd() const override { return Tag_Sliding | Tag_Moving | Tag_BlockMovement | Tag_AbilityAnimation | Tag_FixedVelocity; }
    EStateTag GetCancelTags() const override { return Tag_Moving; }

    void OnActivate() override;

private:
    void OnFinishedAnim();
    void OnInterruptedAnim();

private:
    static constexpr float SLIDE_SPEED = 360.f; 
};
