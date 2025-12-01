#include "pch.h"
#include "Ability_Jump.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"

Ability_Jump::Ability_Jump()
{
}

void Ability_Jump::OnActivate()
{
    Ability::OnActivate();
    
    auto animator = owner->GetComponent<CAnimator>();
    auto rigidbody = owner->GetComponent<CRigidbody>();
    auto velocity = rigidbody->GetVelocity();
    
    if (CPlayer* player = dynamic_cast<CPlayer*>(owner))
    {
        float jumpForce = player->GetJumpForce();
        rigidbody->SetVelocity(Vec2(velocity.x, -jumpForce));
    }
    
    wstring aniName;
    if (IsNearlyEqual(velocity.x,0))
    {
        aniName = AnimKey::JumpStart_Inplace;
    }
    else
    {
        aniName = AnimKey::JumpStart_Moving;
    }
    
    animator->Play(aniName,true, BIND(this, OnFinishedAnim));
    WaitEvent(EGameEvent::VelocityChanged, BIND_EVENT(this, OnVelocityChanged));
    WaitEvent(EGameEvent::Landed, BIND_EVENT(this, EndAbility));
}

void Ability_Jump::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

Vec2 Ability_Jump::GetVelocity()
{
    auto rigidbody = owner->GetComponent<CRigidbody>();
    auto velocity = rigidbody->GetVelocity();
    
    return velocity;
}

void Ability_Jump::OnFinishedAnim()
{
    EndAbility();
}

void Ability_Jump::OnVelocityChanged()
{
    auto animator = owner->GetComponent<CAnimator>();
    auto velocity = GetVelocity();
    
    UINT currentFrame = animator->GetCurrentFrame();
        
    // 제자리에서 뜨는 중
    if (IsNearlyEqual(velocity.x,0))
    {
        animator->Play(AnimKey::JumpStart_Inplace,true,BIND(this, OnFinishedAnim));
        animator->SetCurrentFrame(currentFrame);
    }
    // 옆으로 움직이는 중
    else
    {
        animator->Play(AnimKey::JumpStart_Moving,true, BIND(this, OnFinishedAnim));
        animator->SetCurrentFrame(currentFrame);
    }
}
