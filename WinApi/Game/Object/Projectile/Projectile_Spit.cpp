#include "pch.h"
#include "Projectile_Spit.h"
#include "Game/AnimKey.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Object/Hazard/Hazard_Spike.h"

Projectile_Spit::Projectile_Spit()
{
	name = TEXT("Spit");
}

void Projectile_Spit::Init()
{
	Projectile::Init();

	// Spit 전용 설정
	collider->SetScale(Vec2(30, 30));
	damage = 15.f;

	// Spit 애니메이션 설정
	AddAnimation(AnimKey::ThornBall, TEXT("Animations/Projectile/thorn_projectile_anim.json"), true);
	AddAnimation(AnimKey::ThornBallDestroyed, TEXT("Animations/Projectile/thorn_projectile_destroyed_anim.json"), false);
	AddAnimation(AnimKey::ThornGrowth, TEXT("Animations/Projectile/thorn_projectile_unraveling_anim.json"), false);
	AddAnimation(AnimKey::ThornGrowthDestroyed, TEXT("Animations/Projectile/thorn_projectile_unraveled_destroyed_anim.json"), false);
	
	animator->Play(AnimKey::ThornBall);
}

void Projectile_Spit::OnHitGround(Vec2 hitPos)
{
	if (bIsDestroyed)
		return;
	
	bIsThorn = true;
	groundHitPos = hitPos;
	rigidbody->SetVelocity(Vec2(0, 0));
	rigidbody->UseGravity(false);
	
	// 땅에 닿으면 가시 함정으로 변신
	animator->Play(AnimKey::ThornGrowth, true, [this]()
	{
		bGrowthFinished = true;
		// 2초 후 자동으로 가시 생성
		timerHandle = TIMER->SetTimer([this]()
		{
			OnHitPlayer(nullptr);
		},2.0f);
	});
}

void Projectile_Spit::OnHitWall(Vec2 hitPos)
{
	if (bIsDestroyed)
		return;
	
	// 벽에 닿으면 그냥 파괴 (아무것도 안함)
	animator->Play(AnimKey::ThornBallDestroyed, true, BIND(this, Destroy));
}

void Projectile_Spit::OnHitPlayer(GameObject* player)
{
	if (bIsDestroyed || bSpikeSpawned)
	{
		return;
	}
	
	Projectile::OnHitPlayer(player);
	
	Vec2 spawnPos = collider->GetPos();
	Vec2 colliderHalf = collider->GetScale() * 0.5f;
	spawnPos.y += colliderHalf.y;
	
	if (bGrowthFinished)
	{
		// 변신 완료 -> 가시 함정 스폰
		SpawnSpike(spawnPos);
		SetLifetime(0.2f);
	}
}

void Projectile_Spit::OnDisable()
{
	Projectile::OnDisable();
}

void Projectile_Spit::Release()
{
	Projectile::Release();
}

void Projectile_Spit::OnDamage(GameObject* source, const CombatContext& context)
{
	Projectile::OnDamage(source, context);
	
	if (bIsDestroyed)
		return;
	
	CAMERA->Shake(ShakePreset::Light);
	
	if (bIsThorn)
	{
		animator->Play(AnimKey::ThornGrowthDestroyed, true, BIND(this, Destroy));
	}
	else
	{
		animator->Play(AnimKey::ThornBallDestroyed, true, BIND(this, Destroy));
	}
	
	bIsDestroyed = true;
}

void Projectile_Spit::SpawnSpike(Vec2 spawnPos)
{
	Hazard_Spike* spike = new Hazard_Spike();
	spike->SetPos(spawnPos);
	GetScene()->AddGameObject(spike);
	bSpikeSpawned = true;
}
