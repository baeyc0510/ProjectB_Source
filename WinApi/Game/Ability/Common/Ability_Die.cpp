#include "pch.h"
#include "Ability_Die.h"

#include "Game/Data/AnimKey.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Object/Character/Character.h"

void Ability_Die::OnActivate()
{
    Ability::OnActivate();
    
    Character* character = dynamic_cast<Character*>(GetOwner());
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
    if (Character* character = dynamic_cast<Character*>(GetOwner()))
    {
        character->OnDieStart();
    }
    
    GetStateSystem()->AddTag(Tag_StopVelocity);
    GetAnimator()->Play(AnimKey::Dead, true, BIND(this,OnFinishedAnimation));
    bHasPlayDie = true;
}

void Ability_Die::OnFinishedAnimation()
{
    if (Character* character = dynamic_cast<Character*>(owner))
    {
        character->OnDieComplete();
    }
}
