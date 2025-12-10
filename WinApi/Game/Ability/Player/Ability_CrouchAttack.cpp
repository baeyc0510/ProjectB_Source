#include "pch.h"
#include "Ability_CrouchAttack.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CStatComponent.h"

Ability_CrouchAttack::Ability_CrouchAttack()
{
    AttackData = {
        {50.f, -10.f}, {30.f, 20.f}, VFXKey::AttackHit1
    };
}

void Ability_CrouchAttack::OnActivate()
{
    Ability::OnActivate();

    // AttackData 설정
    float baseAttack = GetStatComponent()->GetCurrent(EStatType::AttackPower);
    AttackData.damage = baseAttack;

    GetAnimator()->Play(AnimKey::CrouchAttack, true, BIND(this, EndAbility), BIND(this, EndAbility));

    WaitEvent(EGameEvent::Input_Crouch_Released, BIND_EVENT(this, OnCrouchReleased));
    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
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

void Ability_CrouchAttack::OnHitCheck()
{
    vector<HitResult> hitResults;
    bool bHit = CombatHelper::ApplyDamageWithAttackData(owner, AttackData, {Monster,Projectile}, hitResults);

    // Play Sound
    if (bHit)
    {
        PlaySFX(SFXKey::PlayerEnemyHit1);
    }
    else
    {
        PlaySFX(SFXKey::PlayerSlashAir1);
    }
}
