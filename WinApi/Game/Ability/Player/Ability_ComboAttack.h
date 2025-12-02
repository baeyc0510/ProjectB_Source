#pragma once
#include "Game/Ability/Ability.h"

class Ability_ComboAttack : public Ability
{
public:
    Ability_ComboAttack();
    
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit | Tag_SpecialAction; }
    EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity |  Tag_AbilityAnimation; }
    EStateTag GetTagsToRemove() const override  { return Tag_None; }
    EStateTag GetCancelTags() const override {return Tag_Crouching;}
    
    void OnActivate() override;
    void OnEnd() override;
    
protected:
    virtual void Attack();
    virtual void OnFinishedAnim();
    virtual void OnComboWindowOpen();
    virtual void OnComboWindowClose();
    virtual void OnInputAttack();
    virtual void OnHitCheck();
    virtual void OnComboCountUpdated(int oldCnt, int newCnt) {}
    
    virtual Vec2 GetTraceOffset();
    virtual Vec2 GetTraceSize();
    virtual wstring GetAnimationName();
    virtual wstring GetVFXName();
    
protected:
    int comboCnt = 0;
    int maxComboCnt = 3;
    bool bCanCombo = false;
    bool bSavedCombo = false;
};
