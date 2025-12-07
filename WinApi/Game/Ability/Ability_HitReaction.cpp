#include "pch.h"
#include "Ability_HitReaction.h"
#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"

void Ability_HitReaction::OnActivate()
{
    Ability::OnActivate();
    
    GetAnimator()->Play(GetHitAnimKey(),true, BIND(this,EndAbility), BIND(this, EndAbility));
    
    // Hitstop + Camera Shake
    TIMER->SetTimeScale(0.0f, 0.05f);
    CAMERA->Shake(ShakePreset::Medium);
    
    WaitEvent(EGameEvent::Recover, BIND_EVENT(this,StopKnockback));
}

void Ability_HitReaction::OnEnd()
{
    Ability::OnEnd();
    
    ClearEventHandles();
    StopKnockback(); // 애니메이션에서 이벤트를 발생시키지 않았더라도 한번 더 멈춤 (안전장치)
}

wstring Ability_HitReaction::GetHitAnimKey() const
{
    return AnimKey::Hit;
}

void Ability_HitReaction::StopKnockback()
{
    auto rigidbody = owner->GetComponent<CRigidbody>();
    rigidbody->SetVelocity(Vec2(0,0));
}

/*~ Ability_ParryHitReaction ~*/

void Ability_ParryHitReaction::OnActivate()
{
    Ability_HitReaction::OnActivate();
    ApplyPushback();
}

void Ability_ParryHitReaction::ApplyPushback()
{
    CRigidbody* rb = owner->GetComponent<CRigidbody>();
    if (!rb)
        return;

    // 현재 방향의 반대로 밀려남
    float dir = static_cast<float>(-owner->GetForward());
    rb->SetVelocity(Vec2(PUSHBACK_FORCE * dir, 0.f));
}
