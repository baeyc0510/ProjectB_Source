#pragma once
#include "Ability_ComboAttack.h"

class Ability_AirAttack : public Ability_ComboAttack
{
public:
    Ability_AirAttack();
    
    StateTag GetRequiredTags() const override { return Tag_Airborne; }
    StateTag GetBlockedTags() const override { return Tag_Grounded | Tag_Hit | Tag_SpecialAction; }
    StateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;
    
protected:
    void OnInputAttack() override;
    void OnComboCountUpdated(int oldCnt, int newCnt) override;
    
    wstring GetAnimationName() override;
    Vec2 GetTraceOffset() override;
    Vec2 GetTraceSize() override;
    
private:
    void OnLanded();

private:
    bool bBlockAirAttack;
    DelegateHandle onLandedHandle;
};
