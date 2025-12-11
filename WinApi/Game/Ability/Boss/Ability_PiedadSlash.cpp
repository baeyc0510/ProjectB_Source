#include "pch.h"
#include "Ability_PiedadSlash.h"

#include "Game/Data/AnimKey.h"
#include "Game/Data/VFXKeys.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Interface/CombatInterface.h"

void Ability_PiedadSlash::OnActivate()
{
	Ability::OnActivate();

	GetAnimator()->Play(AnimKey::BossSlash, true, BIND(this, EndAbility), BIND(this, EndAbility));

	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));

	PlaySFX(SFXKey::PiedadSlash);
}

void Ability_PiedadSlash::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_PiedadSlash::OnHitCheck()
{
	Vec2 offset = GetTraceOffset();
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size = GetTraceSize();

	auto results = COLLISION->BoxTrace(center, size, (UINT)ELayer::Player, true);
	for (auto& result : results)
	{
		GameObject* target = result.collider->GetOwner();
		ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
		if (combat)
		{
			CombatContext context;
			context.damageType = EDamageType::Heavy;
			context.hitResult = result;
			context.value = DAMAGE;
			context.vfxKey = VFXKey::AttackHit1;
			combat->OnDamage(owner, context);
		}
	}
}

Vec2 Ability_PiedadSlash::GetTraceOffset() const
{
	Vec2 offset(80.f, -50.f);
	offset.x *= owner->GetForward();
	return offset;
}

Vec2 Ability_PiedadSlash::GetTraceSize() const
{
	return Vec2(100.f, 80.f);
}
