#pragma once
#include "Game/Ability/Ability.h"

class Ability_UseFlask : public Ability
{
public:
    Ability_UseFlask();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded | Tag_FlaskRemaining; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit | Tag_SpecialAction; }
    StateTag GetTagsToAdd() const override { return Tag_SpecialAction | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void UseFlask();
    
private:
    static constexpr float HEAL_AMOUNT = 50.0f;
};