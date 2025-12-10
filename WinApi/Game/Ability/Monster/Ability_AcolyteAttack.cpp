#include "pch.h"
#include "Ability_AcolyteAttack.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/Util/CombatHelper.h"

void Ability_AcolyteAttack::OnActivate()
{
    Ability::OnActivate();

    CAnimator* animator = owner->GetComponent<CAnimator>();
    animator->Play(AnimKey::Attack, true, BIND(this, EndAbility), BIND(this, EndAbility));
    PlaySFX(SFXKey::AcolytePrepareAttack);

    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
    WaitEvent(EGameEvent::PlaySFX, [this](CGameObject* source)
    {
        PlaySFX(SFXKey::AcolyteReleaseAttack);
    });
}

void Ability_AcolyteAttack::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
}

void Ability_AcolyteAttack::OnHitCheck()
{
    Vec2 offset = GetTraceOffset();
    Vec2 center = owner->GetWorldPos() + offset;

    vector<HitResult> hitResults;
    CombatHelper::ApplyDamageInBox(owner, center, GetTraceSize(), {ELayer::Player}, GetDamage(), hitResults);
}

Vec2 Ability_AcolyteAttack::GetTraceOffset()
{
    auto offset = Vec2(50.0f,-25.0f);
    offset.x *= owner->GetForward();
    return offset;
}

Vec2 Ability_AcolyteAttack::GetTraceSize()
{
    return Vec2(55.0f,25.0f);
}
