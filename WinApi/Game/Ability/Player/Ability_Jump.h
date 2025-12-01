#pragma once
#include "Game/Ability/Ability.h"

class Ability_Jump : public Ability
{
public:
    Ability_Jump();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_BlockMovement; }
    StateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Jumping | Tag_Moving; }
    
    void OnActivate() override;
    void OnEnd() override;
private:
    Vec2 GetVelocity();
    
    void OnFinishedAnim();
    void OnVelocityChanged();
};
