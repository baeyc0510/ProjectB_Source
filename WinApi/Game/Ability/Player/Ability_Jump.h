#pragma once
#include "Game/Ability/Ability.h"

class Ability_Jump : public Ability
{
public:
    Ability_Jump();
    
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_BlockMovement; }
    EStateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Jumping | Tag_Moving; }
    EStateTag GetCancelTags() const override { return Tag_Climbing;}
    
    void OnActivate() override;
    void OnEnd() override;
private:
    Vec2 GetVelocity();
    
    void OnFinishedAnim();
    void OnVelocityChanged();
};
