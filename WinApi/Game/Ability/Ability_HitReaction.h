#pragma once
#include "Ability.h"
#include "Game/AnimKey.h"

class Ability_HitReaction : public Ability
{
public:
    EStateTag GetRequiredTags() const override { return Tag_None; }
    EStateTag GetBlockedTags() const override { return Tag_Invincible | Tag_SpecialAction; }
    EStateTag GetTagsToAdd() const override { return Tag_Hit | Tag_BlockMovement | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Attacking; }

    float GetCooldown() const override {return 0.0f;} // TODO: 캐릭터마다 다르게 적용

    void OnActivate() override;
    void OnEnd() override;

protected:
    virtual wstring GetHitAnimKey() const; 
    
private:
    void StopKnockback();
};

class Ability_ParryHitReaction : public Ability_HitReaction
{
public:
    Ability_ParryHitReaction() 
    {}
    
protected:
    wstring GetHitAnimKey() const override {return AnimKey::ParryHit;}
};
