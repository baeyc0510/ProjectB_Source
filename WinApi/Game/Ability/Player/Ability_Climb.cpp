#include "pch.h"
#include "Ability_Climb.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"

Ability_Climb::Ability_Climb()
{
}

void Ability_Climb::OnActivate()
{
    Ability::OnActivate();

    player = dynamic_cast<CPlayer*>(owner);
    if (!player)
    {
        EndAbility();
        return;
    }
    
    // 중력 설정
    rigidbody = owner->GetComponent<CRigidbody>();
    rigidbody->UseGravity(false);
    rigidbody->SetVelocity(Vec2(0.f, 0.f));

    // 애니메이션 설정
    animator = owner->GetComponent<CAnimator>();
    animator->Stop();
    
    // collider 캐시
    collider = owner->GetComponent<CBoxCollider>();
    
    // 사다리 정보 캐시
    ladderX = player->GetLadderX();
    ladderTopY = player->GetLadderTopY();
    ladderBottomY = player->GetLadderBottomY();

    // 이벤트 대기
    WaitEvent(EGameEvent::Input_Up_Hold, BIND_EVENT(this, OnUpHold));
    WaitEvent(EGameEvent::Input_Down_Hold, BIND_EVENT(this, OnDownHold));
    WaitEvent(EGameEvent::Input_Up_Released, BIND_EVENT(this, OnUpReleased));
    WaitEvent(EGameEvent::Input_Down_Released, BIND_EVENT(this, OnDownReleased));
    WaitEvent(EGameEvent::Input_Jump_Pressed, BIND_EVENT(this, OnJumpPressed));
    WaitEvent(EGameEvent::Landed, BIND_EVENT(this, OnLanded));
}

void Ability_Climb::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();

    // 중력 복원
    rigidbody->UseGravity(true);
    rigidbody->SetVelocity(Vec2(0.f, 0.f));

    // IgnorePlatform 해제
    player->SetIgnorePlatform(0);

    // 애니메이션 정상화
    animator->SetReverse(false);
    
    bClimbBeginHandled = false;
    bHoldDown = false;
    bHoldUp = false;
}

void Ability_Climb::OnUpHold()
{
    if (!bClimbBeginHandled)
    {
        HandleBeginUpward();
    }
    
    bHoldUp = true;

    float centerY = collider->GetPos().y;

    // 상단 경계 체크
    if (centerY <= ladderTopY)
    {
        HandleTopExit();
        return;
    }

    // 위로 이동
    rigidbody->SetVelocity(Vec2(0.f, -CLIMB_SPEED));
    animator->Play(AnimKey::Climbing, false);
    animator->SetReverse(false);
}

void Ability_Climb::OnDownHold()
{
    if (!bClimbBeginHandled)
    {
        HandleBeginDownward();
    }
    
    bHoldDown = true;

    float footY = collider->GetPos().y + collider->GetScale().y * 0.5f;

    // 하단 경계 체크 - 바닥에 도달하면 정지
    if (footY >= ladderBottomY)
    {
        rigidbody->SetVelocity(Vec2(0.f, 0.f));
        animator->Stop();
        return;
    }

    // 아래로 이동
    rigidbody->SetVelocity(Vec2(0.f, CLIMB_SPEED));
    animator->Play(AnimKey::Climbing, false);
    animator->SetReverse(true);

    // 플랫폼 무시 설정
    player->SetIgnorePlatform(player->GetCurrentGroundID());
}

void Ability_Climb::OnUpReleased()
{
    bHoldUp = false;
    CheckAllInputReleased();
}

void Ability_Climb::OnDownReleased()
{
    bHoldDown = false;
    CheckAllInputReleased();
}

void Ability_Climb::OnJumpPressed()
{
    // 탈출
    ExitLadder();
}

void Ability_Climb::CheckAllInputReleased()
{
    // 이동 입력 없을 때 정지
    if (!bHoldUp && !bHoldDown)
    {
        rigidbody->SetVelocity(Vec2(0.f, 0.f));
        animator->Stop();
    }
}

void Ability_Climb::HandleBeginDownward()
{
    // 아래로 진입 시 현재 플랫폼 무시 및 위치 조정
    Vec2 charScale = player->GetCharacterScale();
    player->SetPos(Vec2(ladderX, ladderTopY + charScale.y * 0.5f));
    player->SetIgnorePlatform(player->GetCurrentGroundID());
    
    bClimbBeginHandled = true;
}

void Ability_Climb::HandleBeginUpward()
{
    // X 위치 스냅
    owner->SetPos(Vec2(ladderX, owner->GetPos().y));
    bClimbBeginHandled = true;
}

void Ability_Climb::OnLanded()
{
    ExitLadder();
}

void Ability_Climb::HandleTopExit()
{
    float colHalfY = collider->GetScale().y * 0.5f;
    float offsetY = collider->GetOffset().y;

    // 발이 ladderTopY에 오도록 위치 스냅
    owner->SetPos(Vec2(ladderX, ladderTopY - colHalfY - offsetY));

    ExitLadder();
}

void Ability_Climb::ExitLadder()
{
    EndAbility();
}