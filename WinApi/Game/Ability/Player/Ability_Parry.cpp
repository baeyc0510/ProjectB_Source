#include "pch.h"
#include "Ability_Parry.h"
#include "Game/Data/AnimKey.h"
#include "Game/Data/VFXKeys.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Component/AbilitySystem.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/StateSystem.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Util/CombatHelper.h"
#include "Game/Object/Character/Player.h"

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

    // Player의 OnDamageReceived 델리게이트에 바인딩
    Player* player = dynamic_cast<Player*>(owner);
    if (player)
    {
        onHitHandle = player->OnDamageReceived.SafeAdd([this](GameObject* source, const CombatContext& context) {
            OnHit(source, context);
        });
    }

    WaitEvent(EGameEvent::ParryWindowOpen, BIND_EVENT(this, OnParryWindowOpen));
    WaitEvent(EGameEvent::ParryWindowClose, BIND_EVENT(this, OnParryWindowClose));
}

void Ability_Parry::OnEnd()
{
    Ability::OnEnd();

    // 델리게이트 해제
    onHitHandle.Release();
    ClearEventHandles();

    if (bParrySuccess)
    {
        // 무적 해제
        GetStateSystem()->RemoveTag(Tag_Invincible);
    }

    bParryWindowOpen = false;
    bParrySuccess = false;
    bShouldCounter = false;
}

void Ability_Parry::OnEndParryAnim()
{
    if (bShouldCounter)
    {
        bShouldCounter = false;
        GetAnimator()->Play(AnimKey::ParryCounter, true, BIND(this, EndAbility), BIND(this, OnInterruptedParryAnim));
        WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnCounterHitCheck));
    }
    else
    {
        EndAbility();    
    }
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

void Ability_Parry::OnHit(GameObject* source, const CombatContext& context)
{
    // 패리 윈도우가 아니면 일반 가드
    if (!bParryWindowOpen)
    {
        PlaySFX(SFXKey::PlayerGuard);
        return;
    }

    if (!source)
        return;

    bParrySuccess = true;
    // 무적판정
    GetStateSystem()->AddTag(Tag_Invincible);
    
    // 무거운 공격은 패링반격 X
    if (context.damageType == EDamageType::Heavy || context.damageType == EDamageType::SuperHeavy)
    {
        // TODO: 패링 깨지는 애니메이션 재생
        GetAnimator()->Play(AnimKey::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
        PlaySFX(SFXKey::PlayerParrySuccess);
        return;
    }
    // 가벼운 공격은 패링반격 O
    else
    {
        // 패리 성공 연출 -> 카운터 대기
        bShouldCounter = true;
        GetAnimator()->Play(AnimKey::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
        PlaySFX(SFXKey::PlayerParrySuccess);
        
        // 패리 성공: 상대방 스턴
        if (AbilitySystem* sourceAbilitySystem = source->GetComponent<AbilitySystem>())
        {
            sourceAbilitySystem->TryActivateAbility(EAbility::ParryHit);
        }
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
    data.damageType = EDamageType::Normal;
    data.vfxKey = VFXKey::AttackHit1;

    vector<HitResult> hitResults;
    bool bHit = CombatHelper::ApplyDamageWithAttackData(owner, data, {ELayer::Monster,ELayer::Projectile}, hitResults);

    // 사운드 재생
    if (bHit)
        PlaySFX(SFXKey::PlayerParryCounterHit);
    else
        PlaySFX(SFXKey::PlayerHeavySlash);
}