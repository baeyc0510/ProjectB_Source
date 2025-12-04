#include "pch.h"
#include "Ability_Slide.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"

Ability_Slide::Ability_Slide()
{
}

void Ability_Slide::OnActivate()
{
    const float SLIDE_SPEED = 400.f;

    Ability::OnActivate();

    CAnimator* animator =  owner->GetComponent<CAnimator>();
    animator->Play(GetAnimationName(),true, BIND(this,OnFinishedAnim), BIND(this, OnInterruptedAnim));

    CRigidbody* rigidbody = owner->GetComponent<CRigidbody>();
    Vec2 velocity = rigidbody->GetVelocity();
    velocity.x = SLIDE_SPEED * owner->GetForward();
    rigidbody->SetVelocity(velocity);
}

wstring Ability_Slide::GetAnimationName()
{
    return AnimKey::Slide;
}

void Ability_Slide::OnFinishedAnim()
{
    EndAbility();
}

void Ability_Slide::OnInterruptedAnim()
{
    EndAbility();
}
