#include "pch.h"
#include "CHazard_Vine.h"

CHazard_Vine::CHazard_Vine()
{
	name = TEXT("Vine");
}

void CHazard_Vine::Init()
{
	CHazard::Init();

	// Vine 전용 설정
	collider->SetScale(Vec2(40, 60));
	damage = 10.f;
	lifetime = 5.f;
	damageInterval = 0.5f;

	// TODO: Vine 애니메이션 설정
	// AddAnimation(...)
}

void CHazard_Vine::OnSpawn()
{
	// 생성 애니메이션 재생
	// animator->Play(TEXT("VineSpawn"), true, ...)
}

void CHazard_Vine::OnExpire()
{
	// 사라지는 애니메이션 재생 후 삭제
	// animator->Play(TEXT("VineExpire"), true, ...)
}
