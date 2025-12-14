#pragma once
#include "Game/Ability/Ability.h"

class Ability_UseFlask : public Ability
{
public:
    Ability_UseFlask();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded | Tag_FlaskRemaining; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_BlockAbility | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void UseFlask();

private:
    static constexpr float HEAL_AMOUNT = 150.0f;
};