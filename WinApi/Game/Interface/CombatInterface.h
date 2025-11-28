#pragma once
#include "Game/Enum.h"

struct CombatContext
{
    EDamageType damageType = EDamageType::None;
    float value = 0.0f;
    wstring vfxKey;
    HitResult hitResult;
};

class ICombatInterface
{
public:
    virtual void OnDamage(CGameObject* source, const CombatContext& context) = 0;
};
