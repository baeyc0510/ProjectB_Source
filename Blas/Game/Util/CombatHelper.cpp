#include "pch.h"
#include "CombatHelper.h"
#include "Game/Interface/CombatInterface.h"

bool CombatHelper::ApplyDamageInBox(
    GameObject* source,
    const Vec2& center,
    const Vec2& size,
    std::initializer_list<ELayer> targetLayers,
    float damage,
    vector<HitResult>& outHitResults,
    EDamageType damageType,
    const wchar_t* vfxKey)
{
    outHitResults.clear();

    // 모든 레이어에 대해 BoxTrace 수행 후 결과 합침
    for (ELayer layer : targetLayers)
    {
        vector<HitResult> layerResults = COLLISION->BoxTrace(center, size, static_cast<UINT>(layer), true);
        outHitResults.insert(outHitResults.end(), layerResults.begin(), layerResults.end());
    }

    // 히트된 대상에 데미지 적용
    for (auto& result : outHitResults)
    {
        GameObject* target = result.collider->GetOwner();
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
    GameObject* source,
    const AttackData& attackData,
    std::initializer_list<ELayer> targetLayers,
    vector<HitResult>& outHitResults)
{
    Vec2 offset = attackData.traceOffset;
    offset.x *= source->GetForward();
    Vec2 center = source->GetWorldPos() + offset;

    return ApplyDamageInBox(
        source,
        center,
        attackData.traceSize,
        targetLayers,
        attackData.damage,
        outHitResults,
        attackData.damageType,
        attackData.vfxKey
    );
}
