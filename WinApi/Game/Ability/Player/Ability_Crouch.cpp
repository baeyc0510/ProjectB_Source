#include "pch.h"
#include "Ability_Crouch.h"

#include "Game/Component/CStateSystem.h"

Ability_Crouch::Ability_Crouch()
{
}

void Ability_Crouch::OnActivate()
{
    Ability::OnActivate();
    
    auto animator = owner->GetComponent<CAnimator>();
    animator->Play(L"Crouch",true, nullptr, BIND(this, EndAbility));
    
    // Setup Collider
    auto collider = owner->GetComponent<CCollider>();
    originalColScale = collider->GetScale();
    originalColOffset = collider->GetOffset();
    
    Vec2 crouchScale = originalColScale * Vec2(1.0f,0.5f);
    Vec2 crouchOffset = originalColOffset + crouchScale * Vec2(0.0f,0.5f);
    
    collider->SetScale(crouchScale);
    collider->SetOffset(crouchOffset);
    
    WaitEvent(EGameEvent::Input_Crouch_Released,BIND(this,OnCrouchReleased));
    WaitEvent(EGameEvent::EndCrouch,BIND(this,OnEndCrouch));
}

void Ability_Crouch::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
    
    auto collider = owner->GetComponent<CCollider>();
    collider->SetScale(originalColScale);
    collider->SetOffset(originalColOffset);
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
