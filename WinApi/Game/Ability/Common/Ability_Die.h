#pragma once
#include "Game/Ability/Ability.h"

class Ability_Die : public Ability
{
public:
    Ability_Die() = default;
    ~Ability_Die() override = default;
    
    EStateTag GetBlockedTags() const override { return Tag_Invincible; }
    EStateTag GetTagsToAdd() const override { return Tag_Dead | Tag_BlockMovement | Tag_BlockAbility | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Attacking | Tag_Moving | Tag_SpecialAction |Tag_AbilityAnimation; }
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void PlayDie();
    void OnFinishedAnimation();
    
private:
    bool bHasPlayDie = false;
};
