#pragma once
#include "Game/Ability/Ability.h"
#include "Game/Util/CombatHelper.h"

class Ability_CrouchAttack : public Ability
{
public:
    Ability_CrouchAttack();
    
    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Crouching; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_Crouching | Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnCrouchReleased();
    void OnHitCheck();

private:
    AttackData AttackData;
    bool bCrouchReleased = false;
};
