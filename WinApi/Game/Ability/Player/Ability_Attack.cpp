#include "pch.h"
#include "Ability_Attack.h"

#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

void Ability_Attack::OnActivate()
{
    Ability::OnActivate();
    
    Attack();
    
    WaitEvent(EGameEvent::Input_Attack, BIND(this, OnInputAttack));
    WaitEvent(EGameEvent::HitCheck, BIND(this, OnHitCheck));
    WaitEvent(EGameEvent::ComboWindowOpen, BIND(this, OnComboWindowOpen));
    WaitEvent(EGameEvent::ComboWindowClose, BIND(this, OnComboWindowClose));
}

void Ability_Attack::OnEnd()
{
    Ability::OnEnd();
    
    comboCnt = 0;
    bCanCombo = false;
    bSavedCombo = false;
}

wstring Ability_Attack::GetAnimationName()
{
    if (comboCnt == 0)  return TEXT("Combo1");
    if (comboCnt == 1)  return TEXT("Combo2");
    if (comboCnt == 2)  return TEXT("Combo3");
    
    return TEXT("Combo1");
}

Vec2 Ability_Attack::GetTraceOffset()
{
    if (comboCnt == 0)  return Vec2(40.f,-30.f);
    if (comboCnt == 1)  return Vec2(30.f,-30.f);
    if (comboCnt == 2)  return Vec2(50.f,-30.f);
    
    return Vec2(0,0);
}

Vec2 Ability_Attack::GetTraceSize()
{
    if (comboCnt == 0)  return Vec2(60.f,20.f);
    if (comboCnt == 1)  return Vec2(60.f,20.f);
    if (comboCnt == 2)  return Vec2(80.f,30.f);
    
    return Vec2(0,0);
}

void Ability_Attack::OnComboWindowOpen()
{
    bCanCombo = true;
}

void Ability_Attack::OnComboWindowClose()
{
    bCanCombo = false;
}

void Ability_Attack::OnInputAttack()
{
    if (bCanCombo)
    {
        bSavedCombo = true; 
    }
}

void Ability_Attack::OnFinishedAnim()
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

void Ability_Attack::OnHitCheck()
{
    // Box Trace?
    Vec2 offset = GetTraceOffset();
    offset.x *= owner->GetForward();
    Vec2 center = owner->GetWorldPos() + offset;
    Vec2 size = GetTraceSize();
    
    auto results = COLLISION->BoxTrace(center,size,Layer::Monster, true);
    
    for (auto& result : results)
    {
        // DEBUG
        auto name = result.collider->GetOwner()->GetName();
        Logger::Debug(name + TEXT(" Hit!"));
        
        // Spawn VFX
        CVFX* vfx = VFX->CreateVFX(GetVFXName());
        vfx->SetPos(result.hitCenter);
        vfx->SetForward(owner->GetForward());
        vfx->PlayVFX();
    }
    
    comboCnt = (comboCnt + 1) % 3;  // 콤보 업데이트
}

void Ability_Attack::Attack()
{
    bSavedCombo = false; // 콤보 소모
    
    // 애니메이션 재생
    CAnimator* animator =  owner->GetComponent<CAnimator>();
    animator->Play(GetAnimationName(),true, BIND(this,OnFinishedAnim));
}
wstring Ability_Attack::GetVFXName()
{
    if (comboCnt == 0)  return TEXT("VFX_Attack1");
    if (comboCnt == 1)  return TEXT("VFX_Attack2");
    if (comboCnt == 2)  return TEXT("VFX_Attack3");
    
    return TEXT("VFX_Attack1");
}
