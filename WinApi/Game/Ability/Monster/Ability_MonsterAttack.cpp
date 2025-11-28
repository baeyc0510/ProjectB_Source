#include "pch.h"
#include "Ability_MonsterAttack.h"

#include "Game/Interface/CombatInterface.h"

void Ability_MonsterAttack::OnActivate()
{
    Ability::OnActivate();
    
    CAnimator* animator = owner->GetComponent<CAnimator>();
    animator->Play(TEXT("Attack"),true,BIND(this, EndAbility),BIND(this, EndAbility));
    
    WaitEvent(EGameEvent::HitCheck,BIND(this, OnHitCheck));
}

void Ability_MonsterAttack::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_MonsterAttack::OnHitCheck()
{
    // Box Trace
    Vec2 offset = GetTraceOffset();
    Vec2 center = owner->GetWorldPos() + offset;
    Vec2 size = GetTraceSize();
    
    auto results = COLLISION->BoxTrace(center,size,Layer::Player, true);
    for (auto& result : results)
    {
        CGameObject* target = result.collider->GetOwner();
        ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
        if (combat)
        {
            CombatContext context;
            context.damageType = EDamageType::Slash;
            context.hitResult = result;
            context.value = 10.0f; // TODO: 데미지 공식 처리
            combat->OnDamage(owner,context);
        }
    }
}

Vec2 Ability_MonsterAttack::GetTraceOffset()
{
    auto offset = Vec2(50.0f,-25.0f);
    offset.x *= owner->GetForward();
    return offset;
}

Vec2 Ability_MonsterAttack::GetTraceSize()
{
    return Vec2(55.0f,25.0f);
}
