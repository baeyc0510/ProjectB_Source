#pragma once
#include "Game/Ability/Ability.h"

class Ability_Crouch : public Ability
{
public:
    Ability_Crouch();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_BlockMovement; }
    StateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Crouching | Tag_BlockMovement | Tag_StopVelocity; }
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void OnCrouchReleased();
    void OnEndCrouch();
    
private:
    Vec2 originalColScale;
    Vec2 originalColOffset;
};
