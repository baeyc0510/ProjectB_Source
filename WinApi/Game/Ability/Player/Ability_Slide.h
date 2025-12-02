#pragma once
#include "Game/Ability/Ability.h"

class Ability_Slide : public Ability
{
public:
    Ability_Slide();
    
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_Sliding | Tag_Moving | Tag_BlockMovement | Tag_SpecialAction | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override {return Tag_AbilityAnimation;}
    
    void OnActivate() override;
    
private:
    wstring GetAnimationName();
    void OnFinishedAnim();
    void OnInterruptedAnim();
};
