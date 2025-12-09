#pragma once
#include "Game/Ability/Ability.h"
#include "Game/Util/CombatHelper.h"

class Ability_CrouchAttack : public Ability
{
public:
    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Crouching; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Hit | Tag_SpecialAction; }
    EStateTag GetTagsToAdd() const override { return Tag_Crouching | Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnCrouchReleased();
    void OnHitCheck();

private:
    static const AttackData AttackData;
    bool bCrouchReleased = false;
};
