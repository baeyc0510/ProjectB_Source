#pragma once
#include "Game/Ability/Ability.h"

class Ability_Attack : public Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit; }
    StateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_AbilityAnimation; }
    StateTag GetTagsToRemove() const override  { return Tag_None; }
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void OnFinishedAnim();
    void OnComboWindowOpen();
    void OnComboWindowClose();
    void OnInputAttack();
    void OnHitCheck();
    void Attack();
    
    wstring GetVFXName();
    
    wstring GetAnimationName();
    Vec2 GetTraceOffset();
    Vec2 GetTraceSize();
    
private:
    int comboCnt = 0;
    bool bCanCombo = false;
    bool bSavedCombo = false;
};
