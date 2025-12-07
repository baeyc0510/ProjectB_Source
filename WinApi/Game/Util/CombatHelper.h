#pragma once
#include "Game/Enum.h"

struct FAttackData
{
    Vec2 traceOffset;
    Vec2 traceSize;
    const wchar_t* animKey = nullptr;
    const wchar_t* vfxKey = nullptr;
    float damage = 10.f;
    EDamageType damageType = EDamageType::Slash;
};

class CombatHelper
{
public:
    // BoxTrace로 타겟을 찾아 데미지 적용
    // outHitResults: 히트 결과
    // 반환: 히트 성공 여부
    static bool ApplyDamageInBox(
        CGameObject* source,
        const Vec2& center,
        const Vec2& size,
        ELayer targetLayer,
        float damage,
        vector<HitResult>& outHitResults,
        EDamageType damageType = EDamageType::Slash,
        const wchar_t* vfxKey = nullptr
    );

    // FAttackData 기반 오버로드
    static bool ApplyDamageWithAttackData(
        CGameObject* source,
        const FAttackData& attackData,
        ELayer targetLayer,
        vector<HitResult>& outHitResults
    );
};
