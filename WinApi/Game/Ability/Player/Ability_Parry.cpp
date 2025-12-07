#include "pch.h"
#include "Ability_Parry.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Interface/CombatInterface.h"

Ability_Parry::Ability_Parry()
{
    bParryWindowOpen = false;
    bParrySuccess = false;
    bShouldCounter = false;
}

void Ability_Parry::OnActivate()
{
    Ability::OnActivate();

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
    if (bShouldCounter)
    {
        bShouldCounter = false;
        GetAnimator()->Play(AnimKey::ParryCounter, true, BIND(this, EndAbility), BIND(this, OnInterruptedParryAnim));
        WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnCounterHitCheck));
        return;
    }

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
        return;

    if (!source)
        return;

    // source의 패링 리액션 발동
    if (CAbilitySystem* sourceAbilitySystem = source->GetComponent<CAbilitySystem>())
    {
        sourceAbilitySystem->TryActivateAbility(EAbility::ParryHit);
    }

    // player의 패링 성공 애니메이션 재생
    GetAnimator()->Play(AnimKey::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));

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
}

void Ability_Parry::OnCounterHitCheck()
{
    const float COUNTER_DAMAGE = 10.f;
    const Vec2 TRACE_OFFSET = {50.f, -30.f};
    const Vec2 TRACE_SIZE = {50.f, 30.f};

    Vec2 offset = TRACE_OFFSET;
    offset.x *= owner->GetForward();
    Vec2 center = owner->GetWorldPos() + offset;

    auto results = COLLISION->BoxTrace(center, TRACE_SIZE, ELayer::Monster, true);
    for (auto& result : results)
    {
        CGameObject* target = result.collider->GetOwner();
        ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
        if (combat)
        {
            CombatContext context;
            context.damageType = EDamageType::Slash;
            context.hitResult = result;
            context.value = COUNTER_DAMAGE;
            context.vfxKey = VFXKey::AttackHit1;
            combat->OnDamage(owner, context);
        }
    }
}