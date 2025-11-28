#include "pch.h"
#include "Ability_CrouchAttack.h"

#include "Game/AnimKeys.h"
#include "Game/Component/CAbilitySystem.h"

void Ability_CrouchAttack::OnActivate()
{
    Ability::OnActivate();
    
    auto animator = owner->GetComponent<CAnimator>();
    animator->Play(Anim::CrouchAttack,true,BIND(this,EndAbility),BIND(this,EndAbility));
    
    WaitEvent(EGameEvent::Input_Crouch_Released, BIND(this,OnCrouchReleased));
}

void Ability_CrouchAttack::OnEnd()
{
    Ability::OnEnd();
    
    if (bCrouchReleased)
    {
        abilitySystem->TriggerEvent(EGameEvent::EndCrouch);
    }
    
    ClearEventHandles();
    bCrouchReleased = false;
}

void Ability_CrouchAttack::OnCrouchReleased()
{
    bCrouchReleased = true;
}
