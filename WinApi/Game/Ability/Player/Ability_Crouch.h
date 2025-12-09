#pragma once
#include "Game/Ability/Ability.h"

class Ability_Crouch : public Ability
{
public:
    Ability_Crouch();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_BlockMovement | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_AbilityAnimation | Tag_Crouching | Tag_BlockMovement | Tag_StopVelocity; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnJumpPressed();
    void OnCrouchReleased();
    void OnEndCrouch();
};
