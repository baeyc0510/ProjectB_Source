#include "pch.h"
#include "Ability_Slide.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"

Ability_Slide::Ability_Slide()
{
}

void Ability_Slide::OnActivate()
{
    Ability::OnActivate();

    SFX->PlayOnce(SFXKey::PlayerSlide);
    GetAnimator()->Play(AnimKey::Slide, true, BIND(this, OnFinishedAnim), BIND(this, OnInterruptedAnim));

    Vec2 velocity = GetRigidbody()->GetVelocity();
    velocity.x = SLIDE_SPEED * owner->GetForward();
    GetRigidbody()->SetVelocity(velocity);
}

void Ability_Slide::OnFinishedAnim()
{
    // 끼임 상태면 슬라이드 유지
    if (GetStateSystem()->HasTag(Tag_Squashed))
    {
        GetAnimator()->Play(AnimKey::Slide, true, BIND(this, OnFinishedAnim), BIND(this, OnInterruptedAnim));
        return;
    }
    EndAbility();
}

void Ability_Slide::OnInterruptedAnim()
{
    // 끼임 상태면 슬라이드 유지
    if (GetStateSystem()->HasTag(Tag_Squashed))
    {
        GetAnimator()->Play(AnimKey::Slide, true, BIND(this, OnFinishedAnim), BIND(this, OnInterruptedAnim));
        return;
    }
    EndAbility();
}
