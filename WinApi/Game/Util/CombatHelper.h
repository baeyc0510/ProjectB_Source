#pragma once
#include "Game/Enum.h"
#include <initializer_list>

struct AttackData
{
    Vec2 traceOffset;
    Vec2 traceSize;
    const wchar_t* vfxKey = nullptr;
    float damage = 10.f;
    EDamageType damageType = EDamageType::Slash;
};

class CombatHelper
{
public:
    // BoxTrace로 타겟을 찾아 데미지 적용
    // targetLayers: 검사할 레이어들 (예: {ELayer::Monster, ELayer::Boss})
    // outHitResults: 모든 레이어의 히트 결과를 합친 결과
    // 반환: 히트 성공 여부
    static bool ApplyDamageInBox(
        GameObject* source,
        const Vec2& center,
        const Vec2& size,
        std::initializer_list<ELayer> targetLayers,
        float damage,
        vector<HitResult>& outHitResults,
        EDamageType damageType = EDamageType::Slash,
        const wchar_t* vfxKey = nullptr
    );

    // FAttackData 기반 오버로드
    static bool ApplyDamageWithAttackData(
        GameObject* source,
        const AttackData& attackData,
        std::initializer_list<ELayer> targetLayers,
        vector<HitResult>& outHitResults
    );
};
