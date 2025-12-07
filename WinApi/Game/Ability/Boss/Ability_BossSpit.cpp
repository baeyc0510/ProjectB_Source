#include "pch.h"
#include "Ability_BossSpit.h"

#include "Game/AnimKey.h"
#include "Game/Object/Projectile/CProjectile_Spit.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Ability_BossSpit::OnActivate()
{
	Ability::OnActivate();

	// 발사 횟수 랜덤 결정 (2-3회)
	currentSpitCount = 0;
	maxSpitCount = 2 + (rand() % 2);

	// spit_start 애니메이션 재생
	GetAnimator()->Play(AnimKey::BossSpitStart, true, BIND(this, OnSpitStart), BIND(this, EndAbility));
}

void Ability_BossSpit::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_BossSpit::OnSpitStart()
{
	// spit_loop 시작
	GetAnimator()->Play(AnimKey::BossSpitLoop, true, BIND(this, OnSpitLoop), BIND(this, EndAbility));

	// DoAction 이벤트에서 투사체 발사
	WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, SpawnProjectile));
}

void Ability_BossSpit::OnSpitLoop()
{
	currentSpitCount++;

	if (currentSpitCount < maxSpitCount)
	{
		// 다음 spit_loop 재생
		GetAnimator()->Play(AnimKey::BossSpitLoop, true, BIND(this, OnSpitLoop), BIND(this, EndAbility));
	}
	else
	{
		// 완료 - spit_to_idle 재생
		ClearEventHandles();  // DoAction 이벤트 정리
		GetAnimator()->Play(AnimKey::BossSpitToIdle, true, BIND(this, OnSpitFinish), BIND(this, EndAbility));
	}
}

void Ability_BossSpit::OnSpitFinish()
{
	EndAbility();
}

void Ability_BossSpit::SpawnProjectile()
{
	if (!owner || !owner->GetScene())
		return;

	// 투사체 생성 위치 (보스 입 위치 근처)
	Vec2 spawnOffset(50.f * owner->GetForward(), -60.f);
	Vec2 spawnPos = owner->GetWorldPos() + spawnOffset;

	// 투사체 속도 계산 (각도 기반)
	float angleRad = PROJECTILE_ANGLE * static_cast<float>(M_PI) / 180.f;
	float dirX = static_cast<float>(owner->GetForward());
	Vec2 velocity;
	velocity.x = PROJECTILE_SPEED * cos(angleRad) * dirX;
	velocity.y = -PROJECTILE_SPEED * sin(angleRad);

	// 투사체 생성
	CProjectile_Spit* projectile = new CProjectile_Spit();
	projectile->SetPos(spawnPos);
	projectile->SetProjectileOwner(owner);
	projectile->SetLifetime(10.f);
	owner->GetScene()->AddGameObject(projectile);
	projectile->Launch(velocity);
}
