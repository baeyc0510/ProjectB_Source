#include "pch.h"
#include "Ability_BossSlash.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"

void Ability_BossSlash::OnActivate()
{
	Ability::OnActivate();

	GetAnimator()->Play(AnimKey::BossSlash, true, BIND(this, EndAbility), BIND(this, EndAbility));

	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
}

void Ability_BossSlash::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_BossSlash::OnHitCheck()
{
	Vec2 offset = GetTraceOffset();
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size = GetTraceSize();

	auto results = COLLISION->BoxTrace(center, size, ELayer::Player, true);
	for (auto& result : results)
	{
		CGameObject* target = result.collider->GetOwner();
		ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
		if (combat)
		{
			CombatContext context;
			context.damageType = EDamageType::Slash;
			context.hitResult = result;
			context.value = DAMAGE;
			context.vfxKey = VFXKey::AttackHit1;
			combat->OnDamage(owner, context);
		}
	}
}

Vec2 Ability_BossSlash::GetTraceOffset() const
{
	Vec2 offset(80.f, -50.f);
	offset.x *= owner->GetForward();
	return offset;
}

Vec2 Ability_BossSlash::GetTraceSize() const
{
	return Vec2(100.f, 80.f);
}
