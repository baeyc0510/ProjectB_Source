#pragma once
#include "Game/Ability/Ability.h"

class Ability_Climb : public Ability
{
public:
    Ability_Climb();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_CanClimb; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_BlockMovement | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_BlockMovement | Tag_BlockAbility | Tag_Climbing | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Moving; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void OnUpHold();
    void OnDownHold();
    void OnUpReleased();
    void OnDownReleased();
    void OnJumpPressed();
    void OnGrab();
    void CheckAllInputReleased();
    void HandleBeginDownward();
    void HandleBeginUpward();
    void OnLanded();
    void HandleTopExit();
    void ExitLadder();

private:
    static constexpr float CLIMB_SPEED = 100.f;

    float ladderX = 0.f;
    float ladderTopY = 0.f;
    float ladderBottomY = 0.f;
    class Player* player = nullptr;
    bool bClimbBeginHandled = false;
    bool bHoldDown = false;
    bool bHoldUp = false;
};
