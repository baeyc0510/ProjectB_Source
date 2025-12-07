#include "pch.h"
#include "Ability_AirAttack.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"

const FComboData Ability_AirAttack::AirComboTable[2] = {
    { {30.f, -60.f}, {40.f, 30.f}, AnimKey::AirCombo1, VFXKey::AttackHit1 },
    { {10.f, -40.f}, {50.f, 25.f}, AnimKey::AirCombo2, VFXKey::AttackHit2 },
};

Ability_AirAttack::Ability_AirAttack()
{
    maxComboCnt = 2;
}

void Ability_AirAttack::OnActivate()
{
    Ability_ComboAttack::OnActivate();
    WaitEvent(EGameEvent::Landed, BIND_ARGS(this, OnLanded));
}

void Ability_AirAttack::OnInputAttack()
{
    if (GetStateSystem()->HasTag(Tag_AirAttackExhausted))
    {
        return;
    }
    Ability_ComboAttack::OnInputAttack();
}

void Ability_AirAttack::OnComboCountUpdated(int oldCnt, int newCnt)
{
    if (newCnt == maxComboCnt - 1)
    {
        GetStateSystem()->AddTag(Tag_AirAttackExhausted);
    }
}

const FComboData& Ability_AirAttack::GetComboData() const
{
    return AirComboTable[comboCnt];
}

void Ability_AirAttack::OnLanded(CGameObject* source)
{
    EndAbility();
}
