#include "pch.h"
#include "Ability_Crouch.h"

#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"

Ability_Crouch::Ability_Crouch()
{
}

void Ability_Crouch::OnActivate()
{
    Ability::OnActivate();

    auto animator = owner->GetComponent<CAnimator>();
    animator->Play(L"Crouch",true, nullptr, BIND(this, EndAbility));

    WaitEvent(EGameEvent::Input_Jump_Pressed, BIND_EVENT(this, OnJumpPressed));
    WaitEvent(EGameEvent::Input_Crouch_Released, BIND_EVENT(this,OnCrouchReleased));
    WaitEvent(EGameEvent::EndCrouch, BIND_EVENT(this,OnEndCrouch));
}

void Ability_Crouch::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_Crouch::OnJumpPressed()
{
    if (CPlayer* player = dynamic_cast<CPlayer*>(owner))
    {
        player->SetIgnorePlatform(player->GetCurrentGroundID());
        EndAbility();
    }
}

void Ability_Crouch::OnCrouchReleased()
{
    auto stateSystem = owner->GetComponent<CStateSystem>();
    if (stateSystem->HasTag(Tag_Attacking))
    {
        return;
    }
    
    OnEndCrouch();
}

void Ability_Crouch::OnEndCrouch()
{
    auto animator = owner->GetComponent<CAnimator>();
    animator->Play(L"CrouchUp",true, BIND(this, EndAbility), BIND(this, EndAbility));
}
