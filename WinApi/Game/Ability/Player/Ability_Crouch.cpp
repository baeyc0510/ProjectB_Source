#include "pch.h"
#include "Ability_Crouch.h"

#include "Game/AnimKey.h"
#include "Game/Component/StateSystem.h"
#include "Game/Object/Character/Player.h"

Ability_Crouch::Ability_Crouch()
{
}

void Ability_Crouch::OnActivate()
{
    Ability::OnActivate();

    GetAnimator()->Play(AnimKey::Crouch, true);

    WaitEvent(EGameEvent::Input_Jump_Pressed, BIND_EVENT(this, OnJumpPressed));
    WaitEvent(EGameEvent::Input_Crouch_Released, BIND_EVENT(this, OnCrouchReleased));
    WaitEvent(EGameEvent::EndCrouch, BIND_EVENT(this, OnEndCrouch));
}

void Ability_Crouch::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_Crouch::OnJumpPressed()
{
    if (Player* player = dynamic_cast<Player*>(owner))
    {
        player->SetIgnorePlatform(player->GetCurrentGroundID());
        EndAbility();
    }
}

void Ability_Crouch::OnCrouchReleased()
{
    if (GetStateSystem()->HasTag(Tag_Attacking))
    {
        return;
    }
    OnEndCrouch();
}

void Ability_Crouch::OnEndCrouch()
{
    GetAnimator()->Play(AnimKey::CrouchUp, true, BIND(this, EndAbility), BIND(this, EndAbility));
}