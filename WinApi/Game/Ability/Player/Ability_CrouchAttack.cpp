#include "pch.h"
#include "Ability_CrouchAttack.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Manager/CSFXManager.h"

const AttackData Ability_CrouchAttack::AttackData = {
    {50.f, -10.f}, {30.f, 20.f}, VFXKey::AttackHit1, 10.f
};

void Ability_CrouchAttack::OnActivate()
{
    Ability::OnActivate();

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
    bool bHit = CombatHelper::ApplyDamageWithAttackData(owner, AttackData, ELayer::Monster, hitResults);
    
    // Play Sound
    if (bHit)
    {
        SFX->PlayOnce(SFXKey::PlayerEnemyHit1);
    }
    else
    {
        SFX->PlayOnce(SFXKey::PlayerSlashAir1);
    }
}
