#include "pch.h"
#include "CProjectile_Spit.h"

#include "Game/VFXKeys.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
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

	// TODO: Spit 애니메이션 설정
	// AddAnimation(...)
}

void CProjectile_Spit::OnHitGround(Vec2 hitPos)
{
	// 땅에 닿으면 넝쿨 해저드 생성
	SpawnVineHazard(hitPos);
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

void CProjectile_Spit::SpawnVineHazard(Vec2 pos)
{
	// TODO: Hazard 시스템 구현 후 활성화
	// CHazard_Vine* vine = new CHazard_Vine();
	// vine->SetPos(pos);
	// vine->SetLifetime(5.f);
	// GetScene()->AddGameObject(vine);
}
