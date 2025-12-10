#include "pch.h"
#include "Ability_Parry.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Component/AbilitySystem.h"
#include "Game/Component/StatComponent.h"
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

    PlaySFX(SFXKey::PlayerStartParry);

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

void Ability_Parry::OnHit(GameObject* source)
{
    // 패리 윈도우가 아니면 일반 가드
    if (!bParryWindowOpen)
    {
        PlaySFX(SFXKey::PlayerGuard);
        return;
    }

    if (!source)
        return;

    // 패리 성공: 상대방 스턴
    if (AbilitySystem* sourceAbilitySystem = source->GetComponent<AbilitySystem>())
    {
        sourceAbilitySystem->TryActivateAbility(EAbility::ParryHit);
    }

    // 패리 성공 연출 -> 카운터 윈도우 대기
    GetAnimator()->Play(AnimKey::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
    PlaySFX(SFXKey::PlayerParrySuccess);

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

    // 카운터 윈도우 동안 공격 입력이 있었으면 카운터 공격 실행
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
    bool bHit = CombatHelper::ApplyDamageWithAttackData(owner, data, {ELayer::Monster,ELayer::Projectile}, hitResults);

    // 사운드 재생
    if (bHit)
        PlaySFX(SFXKey::PlayerParryCounterHit);
    else
        PlaySFX(SFXKey::PlayerHeavySlash);
}