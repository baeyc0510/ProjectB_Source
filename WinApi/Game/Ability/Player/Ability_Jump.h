#pragma once
#include "Game/Ability/Ability.h"

class Ability_Jump : Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
    StateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Jumping; }
    StateTag GetTagsToRemove() const override  { return Tag_None; }
    
    void OnActivate() override;
    
private:
    void OnFinishedAnim();
};
