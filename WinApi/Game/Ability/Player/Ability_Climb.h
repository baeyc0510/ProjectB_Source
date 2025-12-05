#pragma once
#include "Game/Ability/Ability.h"

class Ability_Climb : public Ability
{
public:
    Ability_Climb();

    EStateTag GetRequiredTags() const override { return Tag_CanClimb; }
    EStateTag GetBlockedTags() const override { return Tag_BlockMovement | Tag_Hit; }
    EStateTag GetTagsToAdd() const override { return Tag_Climbing | Tag_AbilityAnimation; }
    EStateTag GetCancelTags() const override { return Tag_Moving; }

    void OnActivate() override;
    void OnEnd() override;

private:
    // 입력 처리
    void OnUpHold();
    void OnDownHold();
    void OnUpReleased();
    void OnDownReleased();
    void OnJumpPressed();
    void CheckAllInputReleased();
    
    // 사다리 타기 시작 처리
    void HandleBeginDownward();
    void HandleBeginUpward();
    
    // 착지 처리
    void OnLanded();
    
    // 경계 체크 후 탈출 처리
    void HandleTopExit();
    void ExitLadder();

private:
    static constexpr float CLIMB_SPEED = 100.f;
    
    // 사다리 정보
    float ladderX = 0.f;
    float ladderTopY = 0.f;
    float ladderBottomY = 0.f;
    
    // 캐릭터 상태
    class CPlayer* player;
    class CRigidbody* rigidbody;
    class CAnimator* animator;
    class CBoxCollider* collider;
    
    bool bClimbBeginHandled = false;
    bool bHoldDown = false;
    bool bHoldUp = false;
};
