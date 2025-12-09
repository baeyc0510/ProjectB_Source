#include "pch.h"
#include "Ability_MonsterAttack.h"

#include "Game/AnimKey.h"
#include "Game/Util/CombatHelper.h"

void Ability_MonsterAttack::OnActivate()
{
    Ability::OnActivate();

    CAnimator* animator = owner->GetComponent<CAnimator>();
    animator->Play(AnimKey::Attack, true, BIND(this, EndAbility), BIND(this, EndAbility));

    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
}

void Ability_MonsterAttack::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_MonsterAttack::OnHitCheck()
{
    Vec2 offset = GetTraceOffset();
    Vec2 center = owner->GetWorldPos() + offset;

    vector<HitResult> hitResults;
    CombatHelper::ApplyDamageInBox(owner, center, GetTraceSize(), {ELayer::Player}, GetDamage(), hitResults);
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
