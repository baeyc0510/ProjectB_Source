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
    virtual void OnDamage(GameObject* source, const CombatContext& context) = 0;
    virtual bool IsDead() {return false;}
};
