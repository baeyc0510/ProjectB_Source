#pragma once
#include "Game/Ability/Ability.h"

class Ability_CrouchAttack : public Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_Crouching; }
    StateTag GetBlockedTags() const override { return Tag_Hit | Tag_SpecialAction; }
    StateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void OnCrouchReleased();
    
private:
    bool bCrouchReleased;
};
