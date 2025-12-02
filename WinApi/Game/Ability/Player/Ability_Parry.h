#pragma once
#include "Game/Ability/Ability.h"

class Ability_Parry : public Ability
{
public:
    Ability_Parry();
    
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit | Tag_Sliding; }
    EStateTag GetTagsToAdd() const override { return Tag_Parrying | Tag_SpecialAction | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override {return Tag_Crouching;}
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void OnEndParryAnim();
    void OnInterruptedParryAnim();
    void OnParryWindowOpen();
    void OnParryWindowClose();
    void OnHit(CGameObject* source);
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
