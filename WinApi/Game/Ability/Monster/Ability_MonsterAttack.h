#pragma once
#include "Game/Ability/Ability.h"

class Ability_MonsterAttack : public Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
    StateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    
    void OnActivate() override;
    void OnEnd() override;
    
protected:
    virtual void OnHitCheck();
    virtual Vec2 GetTraceOffset();
    virtual Vec2 GetTraceSize();
};
