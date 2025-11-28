#include "pch.h"
#include "Ability_Jump.h"

Ability_Jump::Ability_Jump()
{
}

void Ability_Jump::OnActivate()
{
    Ability::OnActivate();
    
    auto animator = owner->GetComponent<CAnimator>();
    animator->Play(L"Jump",false,  BIND(this, OnFinishedAnim));
}

void Ability_Jump::OnFinishedAnim()
{
    EndAbility();
}
