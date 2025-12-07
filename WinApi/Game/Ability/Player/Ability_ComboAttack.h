#pragma once
#include "Game/Ability/Ability.h"

struct FComboData
{
    Vec2 traceOffset;
    Vec2 traceSize;
    const wchar_t* animName;
    const wchar_t* vfxName;
};

class Ability_ComboAttack : public Ability
{
public:
    Ability_ComboAttack();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_Airborne | Tag_Hit | Tag_SpecialAction; }
    EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }
    EStateTag GetTagsToRemove() const override { return Tag_None; }
    EStateTag GetCancelTags() const override { return Tag_Crouching; }

    void OnActivate() override;
    void OnEnd() override;

protected:
    /*~ Ability_ComboAttack Interface ~*/
    virtual void Attack();
    virtual void OnFinishedAnim();
    virtual void OnComboWindowOpen();
    virtual void OnComboWindowClose();
    virtual void OnInputAttack();
    virtual void OnHitCheck();
    virtual void OnComboCountUpdated(int oldCnt, int newCnt) {}
    virtual const FComboData& GetComboData() const;

protected:
    static constexpr float BASE_DAMAGE = 10.f;
    static const FComboData ComboTable[3];

    int comboCnt = 0;
    int maxComboCnt = 3;
    bool bCanCombo = false;
    bool bSavedCombo = false;
};
