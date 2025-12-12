#pragma once
#include "Game/Ability/Ability.h"

struct CombatContext;

class Ability_Parry : public Ability
{
public:
    Ability_Parry();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Airborne | Tag_Hit | Tag_BlockMovement; }
    EStateTag GetTagsToAdd() const override { return Tag_Guard | Tag_BlockAbility | Tag_BlockMovement | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Crouching; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnEndParryAnim();
    void OnInterruptedParryAnim();
    void OnParryWindowOpen();
    void OnParryWindowClose();
    void OnHit(GameObject* source, const CombatContext& context);
    void OnCounterHitCheck();

private:
    bool bParryWindowOpen;
    bool bParrySuccess;
    bool bShouldCounter;

    SafeDelegateHandle<GameObject*, const CombatContext&> onHitHandle;
};
