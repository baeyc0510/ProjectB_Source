#include "pch.h"
#include "Ability_Parry.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Util/CombatHelper.h"

namespace
{
    constexpr float COUNTER_DAMAGE_MULTIPLIER = 1.5f;
}

Ability_Parry::Ability_Parry()
{
    bParryWindowOpen = false;
    bParrySuccess = false;
    bShouldCounter = false;
}

void Ability_Parry::OnActivate()
{
    Ability::OnActivate();

    SFX->PlayOnce(SFXKey::PlayerStartParry);

    GetAnimator()->Play(AnimKey::Parry, true, BIND(this, OnEndParryAnim));

    WaitEvent(EGameEvent::Hit, BIND_ARGS(this, OnHit));
    WaitEvent(EGameEvent::ParryWindowOpen, BIND_EVENT(this, OnParryWindowOpen));
    WaitEvent(EGameEvent::ParryWindowClose, BIND_EVENT(this, OnParryWindowClose));
}

void Ability_Parry::OnEnd()
{
    Ability::OnEnd();

    ClearEventHandles();
    bParryWindowOpen = false;
    bParrySuccess = false;
    bShouldCounter = false;
}

void Ability_Parry::OnEndParryAnim()
{
    EndAbility();
}

void Ability_Parry::OnInterruptedParryAnim()
{
    if (!bParrySuccess && !bShouldCounter)
    {
        EndAbility();
    }
}

void Ability_Parry::OnParryWindowOpen()
{
    bParryWindowOpen = true;
}

void Ability_Parry::OnParryWindowClose()
{
    bParryWindowOpen = false;
}

void Ability_Parry::OnHit(CGameObject* source)
{
    if (!bParryWindowOpen)
    {
        SFX->PlayOnce(SFXKey::PlayerGuard);
        return;
    }
    
    if (!source)
        return;

    // source의 패링 리액션 발동
    if (CAbilitySystem* sourceAbilitySystem = source->GetComponent<CAbilitySystem>())
    {
        sourceAbilitySystem->TryActivateAbility(EAbility::ParryHit);
    }
    
    // 플레이어의 리액션
    GetAnimator()->Play(AnimKey::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
    SFX->PlayOnce(SFXKey::PlayerParrySuccess);
    
    onCounterOpenHandle = WaitEvent(EGameEvent::ComboWindowOpen, BIND_EVENT(this, OnCounterOpen));
    onCounterCloseHandle = WaitEvent(EGameEvent::ComboWindowClose, BIND_EVENT(this, OnCounterClose));
    bParrySuccess = true;
}

void Ability_Parry::OnCounterInput()
{
    if (!bParrySuccess)
    {
        assert(!bParryWindowOpen);
        return;
    }
    
    bShouldCounter = true;
}

void Ability_Parry::OnCounterOpen()
{
    onCounterInputHandle = WaitEvent(EGameEvent::Input_Attack_Pressed, BIND_EVENT(this,OnCounterInput));
}

void Ability_Parry::OnCounterClose()
{
    EndWaitEvent(onCounterInputHandle);
    EndWaitEvent(onCounterOpenHandle);
    EndWaitEvent(onCounterCloseHandle);
    
    if (bShouldCounter)
    {
        bShouldCounter = false;
        GetAnimator()->Play(AnimKey::ParryCounter, true, BIND(this, EndAbility), BIND(this, OnInterruptedParryAnim));
        WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnCounterHitCheck));
    }
}

void Ability_Parry::OnCounterHitCheck()
{
    const Vec2 TRACE_OFFSET = {50.f, -30.f};
    const Vec2 TRACE_SIZE = {50.f, 30.f};

    float baseAttack = GetStatComponent()->GetCurrent(EStatType::AttackPower);

    AttackData data;
    data.traceOffset = TRACE_OFFSET;
    data.traceSize = TRACE_SIZE;
    data.damage = baseAttack * COUNTER_DAMAGE_MULTIPLIER;
    data.damageType = EDamageType::Slash;
    data.vfxKey = VFXKey::AttackHit1;

    vector<HitResult> hitResults;
    bool bHit = CombatHelper::ApplyDamageWithAttackData(owner, data, {Monster,Projectile}, hitResults);

    // 사운드 재생
    if (bHit)
        SFX->PlayOnce(SFXKey::PlayerParryCounterHit);
    else
        SFX->PlayOnce(SFXKey::PlayerHeavySlash);
}