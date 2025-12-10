#include "pch.h"
#include "Ability_UseFlask.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/Object/Character/Player.h"

Ability_UseFlask::Ability_UseFlask()
{
}

void Ability_UseFlask::OnActivate()
{
    Ability::OnActivate();

    GetAnimator()->Play(AnimKey::UseFlask, true, BIND(this, EndAbility), BIND(this, EndAbility));
    PlaySFX(SFXKey::PlayerHealing);
    WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, UseFlask));
}

void Ability_UseFlask::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_UseFlask::UseFlask()
{
    if (StatComponent* stat = GetStatComponent())
    {
        int newFlask = stat->GetCurrent(EStatType::Flask) - 1;
        stat->SetCurrent(EStatType::Flask, newFlask);
        
        float newHP = stat->GetCurrent(EStatType::HP) + HEAL_AMOUNT;
        stat->SetCurrent(EStatType::HP, newHP);
    }
}
