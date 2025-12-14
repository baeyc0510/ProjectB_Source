#pragma once
#include "Game/Ability/Ability.h"
#include "Game/Component/AIController.h"

class Ability_StonerAttack : public Ability
{
public:
    /*~ Ability Interface ~*/
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility; }
    EStateTag GetTagsToAdd() const override { return Tag_Attacking | Tag_BlockMovement | Tag_StopVelocity | Tag_AbilityAnimation; }
    float GetCooldown() const override {return 3.0f;}
    
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void ThrowStone();
    
private:
    static constexpr float STONE_SPEED = 200.f;
    static constexpr float STONE_OFFSET_Y = -40.f;
    static constexpr float STONE_OFFSET_X = 5.f;
    AIController* aiController;
};
