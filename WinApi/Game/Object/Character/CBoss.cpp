#include "pch.h"
#include "CBoss.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CBossAIController.h"
#include "Game/Component/CCharacterMovement.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CBoss::CBoss()
{
	name = TEXT("Boss");
}

CBoss::~CBoss()
{
}

void CBoss::Init()
{
	CCharacter::Init();

	// Movement 설정 (보스는 엣지에서 블로킹, 벽에서 블로킹)
	FMovementConfig moveConfig;
	moveConfig.maxSlopeAngle = 50.0f;
	moveConfig.bCanDropThrough = false;
	moveConfig.bBlockAtEdges = true;
	moveConfig.bFlipDirectionAtEdge = false;
	moveConfig.bFlipDirectionAtWall = false;
	movement->SetConfig(moveConfig);

	// Boss AI Controller
	bossAI = new CBossAIController();
	AddChild(bossAI);
}

void CBoss::OnEnable()
{
	CCharacter::OnEnable();
}

void CBoss::Update()
{
	CCharacter::Update();

	UpdateStates();

	if (bHasAppeared)
	{
		UpdateBossAI();
		UpdateBossAnimation();
	}
	else if (!bHasEncountered)
	{
		if (CheckEncounterPlayer())
		{
			bHasEncountered = true;
			TriggerAppearance();
		}
	}
	
	animator->SetDirection(GetForward());
}

void CBoss::Render()
{
	CCharacter::Render();
}

void CBoss::OnDisable()
{
	CCharacter::OnDisable();
	GAMEUI->ShowBossHUD(false);
}

void CBoss::Release()
{
	CCharacter::Release();
}

void CBoss::TriggerAppearance()
{
	if (bHasAppeared)
		return;

	stateSystem->AddTag(Tag_BossAppearing);
	// 보스 별 연출 효과를 Ability에서 구현
	abilitySystem->TryActivateAbility(EAbility::Boss_Appear);
}

void CBoss::OnAppearanceComplete()
{
	bHasAppeared = true;
	stateSystem->RemoveTag(Tag_BossAppearing);
	GAMEUI->ShowBossHUD(true);
	GAMEUI->SetBossHP(currentHP,maxHP);
}

void CBoss::SetCurrentHP(float value)
{
	currentHP = max(0,min(value, maxHP));
	GAMEUI->SetBossHP(currentHP,maxHP);
	
	if (IsNearlyEqual(currentHP,0))
	{
		abilitySystem->TryActivateAbility(EAbility::Die);
	}
	else if (stateSystem->HasTag(Tag_Dead))
	{
		abilitySystem->CancelAbilitiesWithTag(Tag_Dead);
	}
}

void CBoss::SetMaxHP(float value)
{
	maxHP = max(0,value);
}

void CBoss::UpdateBossAnimation()
{
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 기본 Idle
	animator->Play(AnimKey::Idle);
}

void CBoss::UpdateBossAI()
{
	// 이동 불가 상태면 정지
	if (stateSystem->HasAnyTag(Tag_StopVelocity | Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement))
	{
		rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));
		return;
	}

	// AI 컨트롤러에서 공격 선택 및 실행
	if (bossAI && bossAI->HasTarget())
	{
		EAbility nextAttack = bossAI->SelectNextAttack();
		if (nextAttack != EAbility::None)
		{
			abilitySystem->TryActivateAbility(nextAttack);
		}
	}
}

void CBoss::OnDamage(CGameObject* source, const CombatContext& context)
{
	// Trigger Event
	abilitySystem->TriggerEvent(EGameEvent::Hit, source);

	// Spawn VFX
	Vec2 spawnPos = context.hitResult.hitCenter;
	int spawnDirection = source->GetForward();

	// Spawn Hit VFX
	if (!context.vfxKey.empty())
	{
		if (CVFX* vfx = VFX->CreateVFX(context.vfxKey, spawnPos, spawnDirection))
		{
			vfx->PlayVFX();
		}
	}

	if (context.value > 0.0001f)
	{
		// Spawn Blood VFX
		if (CVFX* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
		{
			vfx->PlayVFX();
		}
		
		// Apply Damage
		float newHP = currentHP - context.value;
		SetCurrentHP(newHP);
	}
}

void CBoss::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);
}
