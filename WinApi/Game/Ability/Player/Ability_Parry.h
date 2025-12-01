#pragma once
#include "Game/Ability/Ability.h"

class Ability_Parry : public Ability
{
public:
    Ability_Parry();
    
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit | Tag_Sliding; }
    StateTag GetTagsToAdd() const override { return Tag_Parrying | Tag_SpecialAction | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    StateTag GetCancelTags() const override {return Tag_Crouching;}
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void OnEndParryAnim();
    void OnInterruptedParryAnim();
    void OnParryWindowOpen();
    void OnParryWindowClose();
    void OnHit();
    void OnCounterInput();
    void OnCounterOpen();
    void OnCounterClose();
    void OnCounterHitCheck();
    
private:
    // 상태
    bool bParryWindowOpen;
    bool bParrySuccess;
    bool bShouldCounter;

    DelegateHandle onCounterOpenHandle;
    DelegateHandle onCounterCloseHandle;
    DelegateHandle onCounterInputHandle;
};
