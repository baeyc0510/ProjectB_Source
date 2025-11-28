#include "pch.h"
#include "Ability_HitReaction.h"

#include "Game/Component/CRigidbody.h"

void Ability_HitReaction::OnActivate()
{
    Ability::OnActivate();
    
    CAnimator* animator =  owner->GetComponent<CAnimator>();
    animator->Play(TEXT("Hit"),true, BIND(this,EndAbility), BIND(this, EndAbility));
    
    WaitEvent(EGameEvent::Recover, BIND(this,StopKnockback));
}

void Ability_HitReaction::OnEnd()
{
    Ability::OnEnd();
    
    ClearEventHandles();
    StopKnockback(); // 애니메이션에서 이벤트를 발생시키지 않았더라도 한번 더 멈춤 (안전장치)
}

void Ability_HitReaction::StopKnockback()
{
    auto rigidbody = owner->GetComponent<CRigidbody>();
    rigidbody->SetVelocity(Vec2(0,0));
}
