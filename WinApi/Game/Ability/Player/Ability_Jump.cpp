#include "pch.h"
#include "Ability_Jump.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StatComponent.h"
#include "Game/Object/Character/Player.h"

Ability_Jump::Ability_Jump()
{
}

void Ability_Jump::OnActivate()
{
    Ability::OnActivate();

    PlaySFX(SFXKey::PlayerJump);

    Vec2 velocity = GetRigidbody()->GetVelocity();

    if (StatComponent* stat = GetStatComponent())
    {
        float jumpForce = stat->GetCurrent(EStatType::JumpForce);
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
