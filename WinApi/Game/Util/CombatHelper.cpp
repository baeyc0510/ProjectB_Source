#include "pch.h"
#include "CombatHelper.h"
#include "Game/Interface/CombatInterface.h"

bool CombatHelper::ApplyDamageInBox(
    CGameObject* source,
    const Vec2& center,
    const Vec2& size,
    ELayer targetLayer,
    float damage,
    vector<HitResult>& outHitResults,
    EDamageType damageType,
    const wchar_t* vfxKey)
{
    outHitResults = COLLISION->BoxTrace(center, size, targetLayer, true);

    for (auto& result : outHitResults)
    {
        CGameObject* target = result.collider->GetOwner();
        ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
        if (combat)
        {
            CombatContext context;
            context.damageType = damageType;
            context.hitResult = result;
            context.value = damage;
            if (vfxKey) context.vfxKey = vfxKey;
            combat->OnDamage(source, context);
        }
    }

    return !outHitResults.empty();
}

bool CombatHelper::ApplyDamageWithAttackData(
    CGameObject* source,
    const FAttackData& attackData,
    ELayer targetLayer,
    vector<HitResult>& outHitResults)
{
    Vec2 offset = attackData.traceOffset;
    offset.x *= source->GetForward();
    Vec2 center = source->GetWorldPos() + offset;

    return ApplyDamageInBox(
        source,
        center,
        attackData.traceSize,
        targetLayer,
        attackData.damage,
        outHitResults,
        attackData.damageType,
        attackData.vfxKey
    );
}
