#include "pch.h"
#include "Ability_ComboAttack.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Interface/CombatInterface.h"

const FComboData Ability_ComboAttack::ComboTable[3] = {
    { {40.f, -30.f}, {60.f, 20.f}, AnimKey::Combo1, VFXKey::AttackHit1 },
    { {30.f, -30.f}, {60.f, 20.f}, AnimKey::Combo2, VFXKey::AttackHit2 },
    { {50.f, -30.f}, {80.f, 30.f}, AnimKey::Combo3, VFXKey::AttackHit3 },
};

Ability_ComboAttack::Ability_ComboAttack()
{
}

void Ability_ComboAttack::OnActivate()
{
    Ability::OnActivate();

    // 이벤트 바인딩
    WaitEvent(EGameEvent::Input_Attack_Pressed, BIND_EVENT(this, OnInputAttack));
    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
    WaitEvent(EGameEvent::ComboWindowOpen, BIND_EVENT(this, OnComboWindowOpen));
    WaitEvent(EGameEvent::ComboWindowClose, BIND_EVENT(this, OnComboWindowClose));

    // 공격 1타 시작
    Attack();
}

void Ability_ComboAttack::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();

    comboCnt = 0;
    bCanCombo = false;
    bSavedCombo = false;
}

void Ability_ComboAttack::Attack()
{
    bSavedCombo = false;

    const FComboData& data = GetComboData();
    GetAnimator()->Play(data.animName, true, BIND(this, OnFinishedAnim), BIND(this, OnFinishedAnim));
}

void Ability_ComboAttack::OnComboWindowOpen()
{
    bCanCombo = true;
}

void Ability_ComboAttack::OnComboWindowClose()
{
    bCanCombo = false;
}

void Ability_ComboAttack::OnInputAttack()
{
    if (bCanCombo)
    {
        bSavedCombo = true; 
    }
}

void Ability_ComboAttack::OnFinishedAnim()
{
    if (bSavedCombo)
    {
        Attack();
    }
    else
    {
        EndAbility();
    }
}

void Ability_ComboAttack::OnHitCheck()
{
    const FComboData& data = GetComboData();

    // Box Trace
    Vec2 offset = data.traceOffset;
    offset.x *= owner->GetForward();
    Vec2 center = owner->GetWorldPos() + offset;

    auto results = COLLISION->BoxTrace(center, data.traceSize, ELayer::Monster, true);
    for (auto& result : results)
    {
        CGameObject* target = result.collider->GetOwner();
        ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
        if (combat)
        {
            CombatContext context;
            context.damageType = EDamageType::Slash;
            context.hitResult = result;
            context.value = BASE_DAMAGE;
            context.vfxKey = data.vfxName;
            combat->OnDamage(owner, context);
        }
    }

    // Play Sound
    static const wchar_t* AirSounds[] = {
        SFXKey::PlayerSlashAir1, SFXKey::PlayerSlashAir2,
        SFXKey::PlayerSlashAir3, SFXKey::PlayerSlashAir4
    };
    static const wchar_t* HitSounds[] = {
        SFXKey::PlayerEnemyHit1, SFXKey::PlayerEnemyHit2
    };
    
    const bool bHit = !results.empty();
    if (bHit)
    {
        SFX->PlayOnce(HitSounds[soundIndex % 2]);

        int oldComboCnt = comboCnt;
        comboCnt = (comboCnt + 1) % maxComboCnt;
        OnComboCountUpdated(oldComboCnt, comboCnt);
    }
    else
    {
        SFX->PlayOnce(AirSounds[soundIndex % 4]);
    }

    soundIndex++;
}

const FComboData& Ability_ComboAttack::GetComboData() const
{
    return ComboTable[comboCnt];
}
