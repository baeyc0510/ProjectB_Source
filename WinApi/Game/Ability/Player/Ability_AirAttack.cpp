#include "pch.h"
#include "Ability_AirAttack.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Component/CStateSystem.h"

const ComboData Ability_AirAttack::AirComboTable[2] = {
    {{ {30.f, -60.f}, {40.f, 30.f},VFXKey::AttackHit1, Ability_ComboAttack::BASE_DAMAGE }, AnimKey::AirCombo1},
    {{ {10.f, -40.f}, {50.f, 25.f}, VFXKey::AttackHit2, Ability_ComboAttack::BASE_DAMAGE },AnimKey::AirCombo2},
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

const AttackData& Ability_AirAttack::GetAttackData() const
{
    return AirComboTable[comboCnt].attackData;
}

const wstring& Ability_AirAttack::GetAnimKey() const
{
    return AirComboTable[comboCnt].animKey;
}

void Ability_AirAttack::OnLanded(CGameObject* source)
{
    EndAbility();
}
