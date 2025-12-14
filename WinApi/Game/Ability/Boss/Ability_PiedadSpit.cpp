#include "pch.h"
#include "Ability_PiedadSpit.h"

#include "Game/Data/AnimKey.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Object/Projectile/Projectile_Spit.h"
#include "Game/Object/Character/Boss_TenPiedad.h"
#include "Game/Component/Rigidbody.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Ability_PiedadSpit::OnActivate()
{
	Ability::OnActivate();

	currentSpitCount = 0;
	maxSpitCount = 3;

	// spit_start 애니메이션 재생
	GetAnimator()->Play(AnimKey::BossSpitStart, true, BIND(this, OnSpitStart), BIND(this, EndAbility));

	PlaySFX(SFXKey::PiedadSpitVoice);
}

void Ability_PiedadSpit::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_PiedadSpit::OnSpitStart()
{
	// spit_loop 시작
	GetAnimator()->Play(AnimKey::BossSpitLoop, true, BIND(this, OnSpitLoop), BIND(this, EndAbility));

	// DoAction 이벤트에서 투사체 발사
	WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, SpawnProjectile));
}

void Ability_PiedadSpit::OnSpitLoop()
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

void Ability_PiedadSpit::OnSpitFinish()
{
	EndAbility();
}

void Ability_PiedadSpit::SpawnProjectile()
{
	if (!owner->GetScene())
		return;

	// 투사체 생성 위치 (보스 입 위치 근처)
	Vec2 spawnOffset(50.f * owner->GetForward(), -120.f);
	Vec2 spawnPos = owner->GetWorldPos() + spawnOffset;

	// SpitMinRange 기준으로 목표 거리 계산: [근거리, 중간, 원거리]
	float baseDistance = Boss_TenPiedad::PiedadConfig::Attack::SpitMinRange;
	float targetDistance = baseDistance + (currentSpitCount - 1) * DISTANCE_OFFSET;

	// 목표 거리에 도달하는 속도 계산
	float speed = CalculateSpeedForDistance(targetDistance);

	// 투사체 속도 계산 (각도 기반)
	float angleRad = PROJECTILE_ANGLE * static_cast<float>(M_PI) / 180.f;
	float dirX = static_cast<float>(owner->GetForward());
	Vec2 velocity;
	velocity.x = speed * cos(angleRad) * dirX;
	velocity.y = -speed * sin(angleRad);

	// 투사체 생성
	Projectile_Spit* projectile = new Projectile_Spit();
	projectile->SetPos(spawnPos);
	projectile->SetProjectileOwner(owner);
	projectile->SetLifetime(10.f);
	owner->GetScene()->AddGameObject(projectile);
	projectile->Launch(velocity);
}

float Ability_PiedadSpit::CalculateSpeedForDistance(float targetDistance) const
{
	// 포물선 공식: R = v^2 * sin(2θ) / g
	// 따라서: v = sqrt(R * g / sin(2θ))
	float angleRad = PROJECTILE_ANGLE * static_cast<float>(M_PI) / 180.f;
	float sin2Theta = sin(2.f * angleRad);
	float speed = sqrt(targetDistance * Rigidbody::GRAVITY_CONSTANT / sin2Theta);
	return speed;
}