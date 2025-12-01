#include "pch.h"
#include "Ability_AirAttack.h"
#include "Game/AnimKey.h"
#include "Game/Component/CStateSystem.h"

Ability_AirAttack::Ability_AirAttack()
{
    maxComboCnt = 2;
}

void Ability_AirAttack::OnActivate()
{
    Ability_ComboAttack::OnActivate();
    
    WaitEvent(EGameEvent::Landed,BIND_ARGS(this,OnLanded));
}

void Ability_AirAttack::OnInputAttack()
{
    auto stateSystem = owner->GetComponent<CStateSystem>();
    if (stateSystem->HasTag(Tag_AirAttackExhausted))
    {
        return;
    }

    Ability_ComboAttack::OnInputAttack();
}

void Ability_AirAttack::OnComboCountUpdated(int oldCnt, int newCnt)
{
    if (newCnt == maxComboCnt - 1)
    {
        auto stateSystem = owner->GetComponent<CStateSystem>();
        stateSystem->AddTag(Tag_AirAttackExhausted);
    }
}

wstring Ability_AirAttack::GetAnimationName()
{
    if (comboCnt == 0)  return AnimKey::AirCombo1;
    if (comboCnt == 1)  return AnimKey::AirCombo2;

    return AnimKey::AirCombo1;
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

void Ability_AirAttack::OnLanded(CGameObject* source)
{
    EndAbility();
}
