#include "pch.h"
#include "Hazard_Spike.h"

#include "Game/Data/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Manager/EventBusManager.h"
#include "Game/Util/AnimationHelper.h"


Hazard_Spike::Hazard_Spike()
{
	name = TEXT("Spike");
	damage = 20.0f;
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
		EGameEvent event = AnimationHelper::ToGameEvent(eventName);
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

	// 찌르기 애니메이션 재생
	animator->Play(AnimKey::ThornThrust, true, BIND(this,OnExpire));
	EVENT->OnPlaySFX(this,{SFXKey::HazardBreak});
}

void Hazard_Spike::OnCollisionStay(Collider* other)
{
	if (!bDamageEnabled)
		return;
	
	Hazard::OnCollisionStay(other);
}

void Hazard_Spike::OnExpire()
{
	// 약간의 딜레이 후 삭제
	SetLifetime(0.1f);
}

void Hazard_Spike::OnGameEvent(EGameEvent event)
{
	if (event == EGameEvent::HitCheck)
	{
		bDamageEnabled = true;
		return;
	}
}