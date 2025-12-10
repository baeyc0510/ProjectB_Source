#include "pch.h"
#include "CHazard_Spike.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/VFXKeys.h"
#include "Game/Util/AnimEventHelper.h"
#include "Game/Util/CombatHelper.h"

const AttackData CHazard_Spike::AttackData = {
	{0.f, -40.f}, {15.f, 40.f}, VFXKey::AttackHit1, 10.f, EDamageType::Heavy
};

CHazard_Spike::CHazard_Spike()
{
	name = TEXT("Spike");
}

void CHazard_Spike::Init()
{
	CHazard::Init();

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

void CHazard_Spike::Update()
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

	CHazard::Update();
}

void CHazard_Spike::OnSpawn()
{
	if (!bHasSpawned)
		return;

	// 생성 애니메이션 재생
	animator->Play(AnimKey::ThornThrust, true, BIND(this,OnExpire));
}

void CHazard_Spike::OnExpire()
{
	// 사라지는 애니메이션 재생 후 삭제
	SetLifetime(0.1f);
}

void CHazard_Spike::OnGameEvent(EGameEvent event)
{
	if (event == EGameEvent::HitCheck)
	{
		OnHitCheck();
		return;
	}
}

void CHazard_Spike::OnHitCheck()
{
	vector<HitResult> hitResults;
	CombatHelper::ApplyDamageWithAttackData(this, AttackData, {Player}, hitResults);
}