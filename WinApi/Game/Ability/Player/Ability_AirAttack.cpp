#include "pch.h"
#include "Ability_AirAttack.h"

#include "Game/Component/CStateSystem.h"

Ability_AirAttack::Ability_AirAttack()
{
    maxComboCnt = 2;
    bBlockAirAttack = false;
}

void Ability_AirAttack::OnActivate()
{
    if (bBlockAirAttack)
    {
        EndAbility();
        return;
    }
    
    if (!onLandedHandle.IsValid())
    {
        onLandedHandle = WaitEvent(EGameEvent::Landed, BIND(this,OnLanded, false));
    }
    
    Ability_ComboAttack::OnActivate();
}

void Ability_AirAttack::OnEnd()
{
    Ability_ComboAttack::OnEnd();
    bBlockAirAttack = true;
}

void Ability_AirAttack::OnInputAttack()
{
    if (bBlockAirAttack)
    {
        return;
    }
    
    Ability_ComboAttack::OnInputAttack();
}

void Ability_AirAttack::OnComboCountUpdated(int oldCnt, int newCnt)
{
    if (newCnt == maxComboCnt-1)
    {
        bBlockAirAttack = true;
    }
}

wstring Ability_AirAttack::GetAnimationName()
{
    if (comboCnt == 0)  return TEXT("AirCombo1");
    if (comboCnt == 1)  return TEXT("AirCombo2");
    
    return TEXT("AirCombo1");
}

Vec2 Ability_AirAttack::GetTraceOffset()
{
    Vec2 offset;
    if (comboCnt == 0)  
        offset = {30.f,-60.f};
    if (comboCnt == 1)
        offset = {10.f,-40.f};
    
    offset.x *= owner->GetForward();
    return offset;
}

Vec2 Ability_AirAttack::GetTraceSize()
{
    if (comboCnt == 0)  
        return {40.f,30.f};
    if (comboCnt == 1) 
        return {50.f,25.f};
    
    return {0,0};
}

void Ability_AirAttack::OnLanded()
{
    EndAbility();
    bBlockAirAttack = false;
}
