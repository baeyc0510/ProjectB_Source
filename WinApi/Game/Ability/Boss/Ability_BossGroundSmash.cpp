#include "pch.h"
#include "Ability_BossGroundSmash.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Object/Hazard/Hazard_Spike.h"
#include "Game/Component/BossAIController.h"
#include "Game/Object/Character/Boss.h"

void Ability_BossGroundSmash::OnActivate()
{
	Ability::OnActivate();

	// ground_smash 애니메이션 재생
	GetAnimator()->Play(AnimKey::BossGroundSmash, true, BIND(this, OnSmashToIdle), BIND(this, EndAbility));

	// HitCheck 이벤트에서 임팩트 처리
	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnSmashImpact));
	WaitEvent(EGameEvent::PlaySFX, BIND_EVENT(this, PlaySmashSound));

	PlaySFX(SFXKey::PiedadSmashVoice);
}

void Ability_BossGroundSmash::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_BossGroundSmash::PlaySmashSound()
{
	PlaySFX(SFXKey::PiedadSmash);
}

void Ability_BossGroundSmash::OnSmashImpact()
{
	// Camera Shake
	ShakeCamera(ShakePreset::Boss);
	
	// 임팩트 시 가시 생성
	SpawnSpikes();

	// 근접 범위 데미지 체크
	Vec2 offset(0.f,-50.f);
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size(150.f, 50.f);

	auto results = COLLISION->BoxTrace(center, size, (UINT)ELayer::Player, true);
	for (auto& result : results)
	{
		CGameObject* target = result.collider->GetOwner();
		ICombatInterface* combat = dynamic_cast<ICombatInterface*>(target);
		if (combat)
		{
			CombatContext context;
			context.damageType = EDamageType::SuperHeavy;
			context.hitResult = result;
			context.value = DAMAGE;
			context.vfxKey = VFXKey::AttackHit1;
			combat->OnDamage(owner, context);
		}
	}
}

void Ability_BossGroundSmash::OnSmashToIdle()
{
	// ground_smash_to_idle 애니메이션 재생
	GetAnimator()->Play(AnimKey::BossGroundSmashToIdle, true, BIND(this, EndAbility), BIND(this, EndAbility));

	PlaySFX(SFXKey::PiedadSmashGetUp);
	PlaySFX(SFXKey::PiedadSmashGetUpVoice);
}

void Ability_BossGroundSmash::SpawnSpikes()
{
	if (!owner || !owner->GetScene())
		return;

	int spikeCount = CalculateSpikeCount();
	Vec2 bossPos = owner->GetWorldPos();

	// 양 방향으로 가시 생성
	for (int i = 1; i <= spikeCount; i++)
	{
		float delay = static_cast<float>(i) * SPIKE_SPAWN_DELAY_INTERVAL;

		// 오른쪽 가시
		Hazard_Spike* spikeRight = new Hazard_Spike();
		spikeRight->SetPos(Vec2(bossPos.x + i * SPIKE_SPACING, bossPos.y));
		spikeRight->SetSpawnDelay(delay);
		owner->GetScene()->AddGameObject(spikeRight);

		// 왼쪽 가시
		Hazard_Spike* spikeLeft = new Hazard_Spike();
		spikeLeft->SetPos(Vec2(bossPos.x - i * SPIKE_SPACING, bossPos.y));
		spikeLeft->SetSpawnDelay(delay);
		owner->GetScene()->AddGameObject(spikeLeft);
	}
}

int Ability_BossGroundSmash::CalculateSpikeCount() const
{
	// 보스 AI 컨트롤러에서 플레이어 거리 가져오기
	Boss* boss = dynamic_cast<Boss*>(owner);
	if (!boss || !boss->GetBossAI())
		return MAX_SPIKES_PER_SIDE;

	float distance = boss->GetBossAI()->GetDistanceToTarget();

	// 거리가 가까울수록 가시가 적게 생성 (플레이어가 멀리 도망갔으면 더 많이)
	// 100 이하: 2개, 200 이하: 3개, 300 이하: 4개, 그 이상: 5개
	if (distance <= 100.f) return 2;
	if (distance <= 200.f) return 3;
	if (distance <= 300.f) return 4;
	return MAX_SPIKES_PER_SIDE;
}
