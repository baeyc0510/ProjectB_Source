#include "pch.h"
#include "Ability_Die.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CCharacter.h"

void Ability_Die::OnActivate()
{
    Ability::OnActivate();
    
    CCharacter* character = dynamic_cast<CCharacter*>(GetOwner());
    if (character && character->IsGrounded())
    {
        PlayDie();
    }
    else
    {
        GetAnimator()->Stop();
        WaitEvent(EGameEvent::Landed, BIND_EVENT(this,PlayDie));
    }
}

void Ability_Die::OnEnd()
{
    Ability::OnEnd();
    if (bHasPlayDie)
    {
        GetStateSystem()->RemoveTag(Tag_StopVelocity);    
    }
    bHasPlayDie = false;
}

void Ability_Die::PlayDie()
{
    if (CCharacter* character = dynamic_cast<CCharacter*>(GetOwner()))
    {
        character->OnDieStart();
    }
    
    GetStateSystem()->AddTag(Tag_StopVelocity);
    GetAnimator()->Play(AnimKey::Dead, true, BIND(this,OnFinishedAnimation));
    bHasPlayDie = true;
}

void Ability_Die::OnFinishedAnimation()
{
    if (CCharacter* character = dynamic_cast<CCharacter*>(owner))
    {
        character->OnDieComplete();
    }
}
