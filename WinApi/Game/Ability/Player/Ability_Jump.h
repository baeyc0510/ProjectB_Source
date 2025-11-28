#pragma once
#include "Game/Ability/Ability.h"

class Ability_Jump : Ability
{
public:
    Ability_Jump();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_BlockMovement; }
    StateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Jumping; }
    StateTag GetTagsToRemove() const override  { return Tag_None; }
    
    void OnActivate() override;
    
private:
    void OnFinishedAnim();
};
