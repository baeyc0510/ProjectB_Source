#include "pch.h"
#include "CProjectile_Spit.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Game/Object/Hazard/CHazard_Spike.h"
// #include "Game/Object/Hazard/CHazard_Vine.h"  // 추후 구현

CProjectile_Spit::CProjectile_Spit()
{
	name = TEXT("Spit");
}

void CProjectile_Spit::Init()
{
	CProjectile::Init();

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

void CProjectile_Spit::OnHitGround(Vec2 hitPos)
{
	bIsThorn = true;
	groundHitPos = hitPos;
	rigidbody->SetVelocity(Vec2(0, 0));
	rigidbody->UseGravity(false);
	
	// 땅에 닿으면 변신
	animator->Play(AnimKey::ThornGrowth, true, [this]()
	{
		bGrowthFinished = true;
		// 2초 후 자동으로 가시 생성
		TIMER->SetTimer([this]()
		{
			OnHitPlayer(nullptr);
		},2.0f);
	});
}

void CProjectile_Spit::OnHitWall(Vec2 hitPos)
{
	// 벽에 닿으면 그냥 파괴 (아무것도 안함)
}

void CProjectile_Spit::OnDestroyed()
{
	// 파괴 시 VFX 생성
	// if (CVFX* vfx = VFX->CreateVFX(VFXKey::SpitSplash, GetPos(), GetForward()))
	// {
	//     vfx->PlayVFX();
	// }
	
}

void CProjectile_Spit::OnHitPlayer(CGameObject* player)
{
	if (bSpikeSpawned)
	{
		return;
	}
	
	CProjectile::OnHitPlayer(player);
	
	Vec2 spawnPos = collider->GetPos();
	Vec2 colliderHalf = collider->GetScale() * 0.5f;
	spawnPos.y += colliderHalf.y;
	
	if (bGrowthFinished)
	{
		SpawnSpike(spawnPos);
		SetLifetime(0.2f);
	}
}

void CProjectile_Spit::OnDamage(CGameObject* source, const CombatContext& context)
{
	CProjectile::OnDamage(source, context);
	
	if (bIsThorn) 
	{
		animator->Play(AnimKey::ThornGrowthDestroyed,true,BIND(this, DestroySelf));
	}
	else
	{
		animator->Play(AnimKey::ThornBallDestroyed,true,BIND(this, DestroySelf));
	}
}

void CProjectile_Spit::SpawnSpike(Vec2 spawnPos)
{
	CHazard_Spike* spike = new CHazard_Spike();
	spike->SetPos(spawnPos);
	GetScene()->AddGameObject(spike);
	bSpikeSpawned = true;
}
