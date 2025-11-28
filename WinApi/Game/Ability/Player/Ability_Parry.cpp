#include "pch.h"
#include "Ability_Parry.h"
#include "Game/AnimKeys.h"
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
    
    CAnimator* animator =  owner->GetComponent<CAnimator>();
    animator->Play(Anim::Parry, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
    
    WaitEvent(EGameEvent::Hit, BIND(this,OnHit));
    WaitEvent(EGameEvent::ParryWindowOpen, BIND(this,OnParryWindowOpen));
    WaitEvent(EGameEvent::ParryWindowClose, BIND(this,OnParryWindowClose));
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
        
        CAnimator* animator =  owner->GetComponent<CAnimator>();
        animator->Play(Anim::ParryCounter, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
        
        WaitEvent(EGameEvent::HitCheck,BIND(this,OnCounterHitCheck));
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

void Ability_Parry::OnHit()
{
    if (bParryWindowOpen)
    {
        bParrySuccess = true;
        
        CAnimator* animator =  owner->GetComponent<CAnimator>();
        animator->Play(Anim::ParrySuccess, true, BIND(this, OnEndParryAnim), BIND(this, OnInterruptedParryAnim));
        
        onCounterOpenHandle = WaitEvent(EGameEvent::ComboWindowOpen, BIND(this, OnCounterOpen));
        onCounterCloseHandle = WaitEvent(EGameEvent::ComboWindowClose, BIND(this, OnCounterClose));
    }
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
    onCounterInputHandle = WaitEvent(EGameEvent::Input_Attack_Pressed, BIND(this,OnCounterInput));
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

    auto results = COLLISION->BoxTrace(center, TRACE_SIZE, Layer::Monster, true);
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
            context.vfxKey = TEXT("VFX_Attack1");
            combat->OnDamage(owner, context);
        }
    }
}
