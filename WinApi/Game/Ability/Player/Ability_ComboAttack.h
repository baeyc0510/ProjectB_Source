#pragma once
#include "Game/Ability/Ability.h"
#include "Game/Util/CombatHelper.h"

struct ComboData
{
    AttackData attackData;
    wstring animKey;
};

class Ability_ComboAttack : public Ability
{
public:
    Ability_ComboAttack();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit; }
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
    virtual const AttackData& GetAttackData() const;
    virtual const wstring& GetAnimKey() const;
    
protected:
    vector<ComboData>ComboTable;

    int comboCnt = 0;
    int maxComboCnt = 3;
    int soundIndex = 0;
    
    bool bCanCombo = false;
    bool bSavedCombo = false;
};
