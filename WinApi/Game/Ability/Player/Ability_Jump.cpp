#include "pch.h"
#include "Ability_Jump.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Object/Character/CPlayer.h"

Ability_Jump::Ability_Jump()
{
}

void Ability_Jump::OnActivate()
{
    Ability::OnActivate();

    Vec2 velocity = GetRigidbody()->GetVelocity();

    if (CPlayer* player = dynamic_cast<CPlayer*>(owner))
    {
        float jumpForce = player->GetJumpForce();
        GetRigidbody()->SetVelocity(Vec2(velocity.x, -jumpForce));
    }

    const wchar_t* aniName = IsNearlyEqual(velocity.x, 0)
        ? AnimKey::JumpStart_Inplace
        : AnimKey::JumpStart_Moving;

    GetAnimator()->Play(aniName, true, BIND(this, OnFinishedAnim));
    WaitEvent(EGameEvent::VelocityChanged, BIND_EVENT(this, OnVelocityChanged));
    WaitEvent(EGameEvent::Landed, BIND_EVENT(this, EndAbility));
}

void Ability_Jump::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_Jump::OnFinishedAnim()
{
    EndAbility();
}

void Ability_Jump::OnVelocityChanged()
{
    Vec2 velocity = GetRigidbody()->GetVelocity();
    UINT currentFrame = GetAnimator()->GetCurrentFrame();

    const wchar_t* aniName = IsNearlyEqual(velocity.x, 0)
        ? AnimKey::JumpStart_Inplace
        : AnimKey::JumpStart_Moving;

    GetAnimator()->Play(aniName, true, BIND(this, OnFinishedAnim));
    GetAnimator()->SetCurrentFrame(currentFrame);
}
