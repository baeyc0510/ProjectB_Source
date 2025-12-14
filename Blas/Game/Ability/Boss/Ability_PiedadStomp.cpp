#include "pch.h"
#include "Ability_PiedadStomp.h"

#include "Game/Data/AnimKey.h"
#include "Game/Data/VFXKeys.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Object/Character/Boss_TenPiedad.h"
#include "Game/Object/Hazard/Hazard_Spike.h"

void Ability_PiedadStomp::OnActivate()
{
	Ability::OnActivate();

	GetAnimator()->Play(AnimKey::BossStomp, true, BIND(this, EndAbility), BIND(this, EndAbility));

	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
	PlaySFX(SFXKey::PiedadStomp);
}

void Ability_PiedadStomp::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_PiedadStomp::OnHitCheck()
{
	// Camera Shake
	ShakeCamera(ShakePreset::Heavy);
	
	Vec2 offset = GetTraceOffset();
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size = GetTraceSize();

	auto results = COLLISION->BoxTrace(center, size, (UINT)ELayer::Player, true);
	for (auto& result : results)
	{
		GameObject* player = result.collider->GetOwner();
		
		ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
		if (combat)
		{
			CombatContext context;
			context.damageType = EDamageType::Heavy;  // 가드해도 밀려남
			context.hitResult = result;
			context.value =  DAMAGE_MULT * GetStatComponent()->GetCurrent(EStatType::AttackPower);;
			context.vfxKey = VFXKey::AttackHit1;
			combat->OnDamage(owner, context);
		}
		
		// 가시 생성
		SpawnHazard();
	}
}

void Ability_PiedadStomp::SpawnHazard()
{
	Vec2 bossPos = owner->GetWorldPos();
	float stompRange = Boss_TenPiedad::PiedadConfig::Attack::StompRange;
	Vec2 spawnPos = bossPos + Vec2(stompRange,0) * owner->GetForward();
	
	// 가시 생성
	Hazard_Spike* spikeRight = new Hazard_Spike();
	spikeRight->SetPos(spawnPos);
	spikeRight->SetSpawnDelay(0.5f);
	owner->GetScene()->AddGameObject(spikeRight);
}

Vec2 Ability_PiedadStomp::GetTraceOffset() const
{
	Vec2 offset(90.f, -40.f);
	offset.x *= owner->GetForward();
	return offset;
}

Vec2 Ability_PiedadStomp::GetTraceSize() const
{
	return Vec2(80.f, 60.f);
}
