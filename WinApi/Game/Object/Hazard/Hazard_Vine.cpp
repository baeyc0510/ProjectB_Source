#include "pch.h"
#include "Hazard_Vine.h"

Hazard_Vine::Hazard_Vine()
{
	name = TEXT("Vine");
}

void Hazard_Vine::Init()
{
	Hazard::Init();

	// Vine 전용 설정
	collider->SetScale(Vec2(40, 60));
	damage = 10.f;
	damageInterval = 0.5f;

	// TODO: Vine 애니메이션 설정
	// AddAnimation(...)
}

void Hazard_Vine::OnSpawn()
{
	// 생성 애니메이션 재생
	// animator->Play(TEXT("VineSpawn"), true, ...)
}