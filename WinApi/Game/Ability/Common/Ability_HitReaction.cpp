#include "pch.h"
#include "Ability_HitReaction.h"
#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Object/Character/CCharacter.h"
#include "Game/Object/Character/CPlayer.h"

void Ability_HitReaction::OnActivate()
{
    Ability::OnActivate();

    GetAnimator()->Play(GetHitAnimKey(),true, BIND(this,OnFinishedReaction), BIND(this, OnInterruptedReaction));

    // Hitstop + Camera Shake
    SetTimeScale(0.0f, 0.05f);
    ShakeCamera(ShakePreset::Medium);
    
    // 리커버
    WaitEvent(EGameEvent::Recover, BIND_EVENT(this,OnRecover));
}

void Ability_HitReaction::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

wstring Ability_HitReaction::GetHitAnimKey() const
{
    return AnimKey::Hit;
}

void Ability_HitReaction::OnFinishedReaction()
{
    EndAbility();
}

void Ability_HitReaction::OnInterruptedReaction()
{
    EndAbility();
}

void Ability_HitReaction::OnRecover()
{
    auto rigidbody = owner->GetComponent<CRigidbody>();
    rigidbody->SetVelocity(Vec2(0,0));
}

/*~ Ability_ParryHitReaction ~*/

void Ability_ParryHitReaction::OnActivate()
{
    Ability_HitReaction::OnActivate();
}

void Ability_PlayerPushback::OnActivate()
{
    Ability_HitReaction::OnActivate();
    GetRigidbody()->SetGravityScale(0.9f);
    WaitEvent(EGameEvent::Landed, BIND_EVENT(this,OnLanded));
    PlaySFX(SFXKey::PlayerPushback);
}

void Ability_PlayerPushback::OnEnd()
{
    Ability_HitReaction::OnEnd();
    bIsGettingUp = false;
    GetRigidbody()->SetGravityScale(CPlayer::PLAYER_GRAVITY_SCALE);
}

wstring Ability_PlayerPushback::GetHitAnimKey() const
{
    return AnimKey::Pushback;
}

void Ability_PlayerPushback::OnFinishedReaction()
{
    auto velocity = GetRigidbody()->GetVelocity();
    velocity.x = 0;
    GetRigidbody()->SetVelocity(velocity);
    
    CCharacter* character = dynamic_cast<CCharacter*>(owner);
    if (!character)
        return;
    
    if (!bIsGettingUp && character->IsGrounded())
    {
        OnLanded();
    }
}

void Ability_PlayerPushback::OnInterruptedReaction()
{
    if (bIsGettingUp)
        return;
    
    Ability_HitReaction::OnInterruptedReaction();
}

void Ability_PlayerPushback::OnLanded()
{
    EndAbility();
    
    // if (bIsGettingUp)
    //     return;
    //
    // GetRigidbody()->SetVelocity(Vec2(0,0));
    // bIsGettingUp = true;
    // GetAnimator()->Play(AnimKey::Pushback_Land,false, BIND(this, EndAbility), BIND(this,EndAbility));
}
