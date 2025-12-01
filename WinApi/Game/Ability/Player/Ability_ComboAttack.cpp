#include "pch.h"
#include "Ability_ComboAttack.h"
#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"

Ability_ComboAttack::Ability_ComboAttack()
{
}

void Ability_ComboAttack::OnActivate()
{
    Ability::OnActivate();

    // 이벤트 바인딩
    WaitEvent(EGameEvent::Input_Attack_Pressed, BIND_ARGS(this, OnInputAttack));
    WaitEvent(EGameEvent::HitCheck, BIND_ARGS(this, OnHitCheck));
    WaitEvent(EGameEvent::ComboWindowOpen, BIND_ARGS(this, OnComboWindowOpen));
    WaitEvent(EGameEvent::ComboWindowClose, BIND_ARGS(this, OnComboWindowClose));

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
    bSavedCombo = false; // 콤보 소모
    
    // 애니메이션 재생
    CAnimator* animator =  owner->GetComponent<CAnimator>();
    animator->Play(GetAnimationName(),true, BIND(this,OnFinishedAnim), BIND(this,OnFinishedAnim));
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
    const float BASE_DAMAGE = 10.f;

    // Box Trace
    Vec2 offset = GetTraceOffset();
    Vec2 center = owner->GetWorldPos() + offset;
    Vec2 size = GetTraceSize();
    
    auto results = COLLISION->BoxTrace(center,size,Layer::Monster, true);
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
            context.vfxKey = GetVFXName();
            combat->OnDamage(owner, context);
        }
    }
    
    const bool bHit = !results.empty();
    if (bHit)
    {
        int oldComboCnt = comboCnt;
        comboCnt = (comboCnt + 1) % maxComboCnt;  // 콤보 업데이트
        OnComboCountUpdated(oldComboCnt, comboCnt);
    }
}

Vec2 Ability_ComboAttack::GetTraceOffset()
{
    Vec2 offset;
    if (comboCnt == 0)  
        offset = {40.f,-30.f};
    if (comboCnt == 1)
        offset = {30.f,-30.f};
    if (comboCnt == 2)
        offset = {50.f,-30.f};
    
    offset.x *= owner->GetForward();
    return offset;
}

Vec2 Ability_ComboAttack::GetTraceSize()
{
    if (comboCnt == 0)  
        return {60.f,20.f};
    if (comboCnt == 1) 
        return {60.f,20.f};
    if (comboCnt == 2)  
        return {80.f,30.f};
    
    return {0,0};
}

wstring Ability_ComboAttack::GetAnimationName()
{
    if (comboCnt == 0)  return AnimKey::Combo1;
    if (comboCnt == 1)  return AnimKey::Combo2;
    if (comboCnt == 2)  return AnimKey::Combo3;

    return AnimKey::Combo1;
}

wstring Ability_ComboAttack::GetVFXName()
{
    if (comboCnt == 0)  return VFXKey::AttackHit1;
    if (comboCnt == 1)  return VFXKey::AttackHit2;
    if (comboCnt == 2)  return VFXKey::AttackHit3;
    
    return VFXKey::AttackHit1;
}
