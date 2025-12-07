#pragma once
#include "Ability_ComboAttack.h"

class Ability_AirAttack : public Ability_ComboAttack
{
public:
    Ability_AirAttack();

    EStateTag GetRequiredTags() const override { return Tag_Airborne; }
    EStateTag GetBlockedTags() const override { return Tag_Grounded | Tag_Hit | Tag_SpecialAction | Tag_AirAttackExhausted; }
    EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override {return Tag_Jumping;}

protected:
    void OnActivate() override;
    void OnInputAttack() override;
    void OnComboCountUpdated(int oldCnt, int newCnt) override;
    const FComboData& GetComboData() const override;

private:
    void OnLanded(CGameObject* source);

private:
    static const FComboData AirComboTable[2];
};
