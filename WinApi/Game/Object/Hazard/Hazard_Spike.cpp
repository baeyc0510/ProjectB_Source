#include "pch.h"
#include "Hazard_Spike.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/VFXKeys.h"
#include "Game/Util/AnimEventHelper.h"
#include "Game/Util/CombatHelper.h"

const AttackData Hazard_Spike::AttackData = {
	{0.f, -40.f}, {15.f, 40.f}, VFXKey::AttackHit1, 10.f, EDamageType::Heavy
};

Hazard_Spike::Hazard_Spike()
{
	name = TEXT("Spike");
}

void Hazard_Spike::Init()
{
	Hazard::Init();

	// Spike 전용 설정
	collider->SetScale(Vec2(30, 80));
	collider->SetOffset(Vec2(0, -40));
	damage = 15.f;
	damageInterval = 0.3f;
	
	// animator 설정
	animator->OnFrameEvent.Add([this](const wstring& eventName)
	{
		EGameEvent event = AnimEventHelper::ToGameEvent(eventName);
		OnGameEvent(event);
	});

	AddAnimation(AnimKey::ThornThrust, TEXT("Animations/Projectile/thorns_tower_anim.json"), false);
}

void Hazard_Spike::Update()
{
	// 지연 스폰 처리
	if (!bHasSpawned)
	{
		currentDelay += DT;
		if (currentDelay >= spawnDelay)
		{
			bHasSpawned = true;
			OnSpawn();
		}
		return;  // 스폰 전에는 업데이트 안함
	}

	Hazard::Update();
}

void Hazard_Spike::OnSpawn()
{
	if (!bHasSpawned)
		return;

	// 생성 애니메이션 재생
	animator->Play(AnimKey::ThornThrust, true, BIND(this,OnExpire));
}

void Hazard_Spike::OnExpire()
{
	// 사라지는 애니메이션 재생 후 삭제
	SetLifetime(0.1f);
}

void Hazard_Spike::OnGameEvent(EGameEvent event)
{
	if (event == EGameEvent::HitCheck)
	{
		OnHitCheck();
		return;
	}
}

void Hazard_Spike::OnHitCheck()
{
	vector<HitResult> hitResults;
	CombatHelper::ApplyDamageWithAttackData(this, AttackData, {ELayer::Player}, hitResults);
}