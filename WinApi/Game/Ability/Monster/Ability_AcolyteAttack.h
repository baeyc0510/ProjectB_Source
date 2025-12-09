#pragma once
#include "Game/Ability/Ability.h"

class Ability_AcolyteAttack : public Ability
{
public:
    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;

protected:
    /*~ Ability_MonsterAttack Interface ~*/
    virtual void OnHitCheck();
    virtual Vec2 GetTraceOffset();
    virtual Vec2 GetTraceSize();
    virtual float GetDamage() { return 10.f; }
};
