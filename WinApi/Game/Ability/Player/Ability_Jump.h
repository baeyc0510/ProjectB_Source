#pragma once
#include "Game/Ability/Ability.h"

class Ability_Jump : public Ability
{
public:
    Ability_Jump();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_BlockMovement; }
    EStateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Jumping | Tag_Moving; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnFinishedAnim();
    void OnVelocityChanged();
};
