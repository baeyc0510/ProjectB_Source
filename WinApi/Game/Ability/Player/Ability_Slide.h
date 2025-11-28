#pragma once
#include "Game/Ability/Ability.h"

class Ability_Slide : public Ability
{
public:
    Ability_Slide();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
    StateTag GetTagsToAdd() const override { return Tag_Sliding | Tag_BlockMovement | Tag_SpecialAction | Tag_AbilityAnimation; }
    StateTag GetCancelTags() const override {return Tag_AbilityAnimation;}
    
    void OnActivate() override;
    
private:
    wstring GetAnimationName();
    void OnFinishedAnim();
    void OnInterruptedAnim();
};
