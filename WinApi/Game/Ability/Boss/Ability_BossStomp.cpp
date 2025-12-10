#include "pch.h"
#include "Ability_BossStomp.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"

void Ability_BossStomp::OnActivate()
{
	Ability::OnActivate();

	GetAnimator()->Play(AnimKey::BossStomp, true, BIND(this, EndAbility), BIND(this, EndAbility));

	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
	PlaySFX(SFXKey::PiedadStomp);
}

void Ability_BossStomp::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_BossStomp::OnHitCheck()
{
	// Camera Shake
	ShakeCamera(ShakePreset::Heavy);
	
	Vec2 offset = GetTraceOffset();
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size = GetTraceSize();

	auto results = COLLISION->BoxTrace(center, size, (UINT)ELayer::Player, true);
	for (auto& result : results)
	{
		CGameObject* player = result.collider->GetOwner();
		
		ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
		if (combat)
		{
			CombatContext context;
			context.damageType = EDamageType::Heavy;  // 가드해도 밀려남
			context.hitResult = result;
			context.value = DAMAGE;
			context.vfxKey = VFXKey::AttackHit1;
			combat->OnDamage(owner, context);
		}
		
		// 가시 생성
		SpawnHazard();
	}
}

void Ability_BossStomp::SpawnHazard()
{
	// TODO: 착지 예상 위치에 가시 생성 (추후 Hazard 시스템 구현 후)
	// Vec2 landingPos = PredictLandingPosition(player);
	// ScheduleSpikeSpawn(landingPos);
}

void Ability_BossStomp::ScheduleSpikeSpawn(Vec2 landingPos)
{
	// TODO: Hazard 시스템 구현 후 가시 생성 로직 추가
	// 플레이어 착지 예상 위치에 가시를 생성
}

Vec2 Ability_BossStomp::GetTraceOffset() const
{
	Vec2 offset(90.f, -40.f);
	offset.x *= owner->GetForward();
	return offset;
}

Vec2 Ability_BossStomp::GetTraceSize() const
{
	return Vec2(80.f, 60.f);
}
