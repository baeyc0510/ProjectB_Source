#include "pch.h"
#include "Ability_UseFlask.h"

#include "Game/AnimKey.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Object/Character/CPlayer.h"

Ability_UseFlask::Ability_UseFlask()
{
}

void Ability_UseFlask::OnActivate()
{
    Ability::OnActivate();

    GetAnimator()->Play(AnimKey::UseFlask, true, BIND(this, EndAbility), BIND(this, EndAbility));
    SFX->PlayOnce(SFXKey::PlayerHealing);
    WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, UseFlask));
}

void Ability_UseFlask::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_UseFlask::UseFlask()
{
    if (CPlayer* player = dynamic_cast<CPlayer*>(owner))
    {
        int newFlask = player->GetCurrentFlask() - 1;
        player->SetCurrentFlask(newFlask);
        
        float newHP = player->GetCurrentHP() + HEAL_AMOUNT;
        player->SetCurrentHP(newHP);
    }
}
