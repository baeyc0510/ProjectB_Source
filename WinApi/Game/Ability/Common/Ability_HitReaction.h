#pragma once
#include "Game/Data/AnimKey.h"
#include "Game/Ability/Ability.h"

class Ability_HitReaction : public Ability
{
public:
    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_None; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_Invincible; }
    EStateTag GetTagsToAdd() const override { return Tag_Hit | Tag_BlockMovement | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Attacking; }
    float GetCooldown() const override { return 0.0f; }

    void OnActivate() override;
    void OnEnd() override;

protected:
    virtual wstring GetHitAnimKey() const;
    virtual void OnFinishedReaction();
    virtual void OnInterruptedReaction();
    virtual void OnRecover();
};

class Ability_ParryHitReaction : public Ability_HitReaction
{
public:
    void OnActivate() override;

protected:
    /*~ Ability_HitReaction Interface ~*/
    wstring GetHitAnimKey() const override { return AnimKey::ParryHit; }
};

class Ability_PlayerPushback : public Ability_HitReaction
{
protected:
    void OnActivate() override;
    void OnEnd() override;
    
    wstring GetHitAnimKey() const override;
    void OnFinishedReaction() override;
    void OnInterruptedReaction() override;
private:
    void OnLanded();
    
private:
    bool bIsGettingUp = false;
};
