#include "pch.h"
#include "CEnemy.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAIController.h"
#include "Game/Component/CCharacterMovement.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CEnemy::CEnemy()
{
	name = TEXT("몬스터");
}

CEnemy::~CEnemy()
{
}

void CEnemy::Init()
{
	CCharacter::Init();

	// Movement (AI 설정: 엣지 블로킹, 방향 전환)
	FMovementConfig moveConfig;
	moveConfig.maxSlopeAngle = 50.0f;
	moveConfig.bCanDropThrough = false;
	moveConfig.bBlockAtEdges = true;
	moveConfig.bFlipDirectionAtEdge = true;
	moveConfig.bFlipDirectionAtWall = true;
	movement->SetConfig(moveConfig);

	// AIController
	aiController = new CAIController();
	AddChild(aiController);
}

void CEnemy::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(AnimKey::Idle);
}

void CEnemy::Update()
{
	CCharacter::Update();

	UpdateStates();
	HandleMovementEvents();
	UpdateAIMovement();
	UpdateAnimation();
}

void CEnemy::Render()
{
	CCharacter::Render();
}

void CEnemy::OnDisable()
{
	CCharacter::OnDisable();
}

void CEnemy::Release()
{
	CCharacter::Release();
}

void CEnemy::UpdateAnimation()
{
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 이동 중이면 Walk, 아니면 Idle
	if (stateSystem->HasAnyTag(Tag_AIPatrol | Tag_AIChase))
	{
		animator->Play(AnimKey::Walk);
	}
	else
	{
		animator->Play(AnimKey::Idle);
	}
}

void CEnemy::HandleMovementEvents()
{
	if (!movement)
		return;

	// 순찰 중 벽 충돌 시 방향 전환
	if (stateSystem->HasTag(Tag_AIPatrol))
	{
		if (movement->DidHitWall())
		{
			int wallDir = movement->GetWallHitDirection();
			int patrolDir = aiController->GetPatrolDirection();
			if (wallDir == patrolDir)
			{
				aiController->FlipPatrolDirection();
			}
		}

		if (movement->DidReachEdge())
		{
			aiController->FlipPatrolDirection();
		}
	}
}

void CEnemy::UpdateAIMovement()
{
	// 이동 불가 상태 (CCharacter::Update에서 Tag_StopVelocity 처리됨)
	if (stateSystem->HasAnyTag(Tag_StopVelocity | Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement))
	{
		rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));
		return;
	}

	const FAIConfig& config = aiController->GetConfig();
	float currentX = GetPos().x;
	float safeMinX = aiController->GetSafeMinX();
	float safeMaxX = aiController->GetSafeMaxX();

	// 순찰 모드
	if (stateSystem->HasTag(Tag_AIPatrol))
	{
		int dir = aiController->GetPatrolDirection();

		// 안전 범위 경계 체크 - 방향 전환
		if ((dir > 0 && currentX >= safeMaxX) || (dir < 0 && currentX <= safeMinX))
		{
			aiController->FlipPatrolDirection();
			dir = aiController->GetPatrolDirection();
		}

		SetForward(dir);
		animator->SetDirection(dir);
		rigidbody->SetVelocity(Vec2(config.patrolSpeed * dir, rigidbody->GetVelocity().y));
	}
	// 추격 모드
	else if (stateSystem->HasTag(Tag_AIChase))
	{
		int dir = aiController->GetDirectionToTarget();

		// 공격 범위 내면 정지 + 공격 시도
		if (aiController->IsTargetInAttackRange())
		{
			rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));
			abilitySystem->TryActivateAbility(EAbility::Attack);
		}
		// 안전 범위 밖이면 정지
		else if ((dir > 0 && currentX >= safeMaxX) || (dir < 0 && currentX <= safeMinX))
		{
			rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));
		}
		else
		{
			SetForward(dir);
			animator->SetDirection(dir);
			rigidbody->SetVelocity(Vec2(config.chaseSpeed * dir, rigidbody->GetVelocity().y));
		}
	}
	// 기본 상태 - 정지
	else
	{
		rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));
	}
}

void CEnemy::OnDamage(CGameObject* source, const CombatContext& context)
{
	// DEBUG
	Logger::Debug(name + TEXT(" Hit!"));

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

		// Hit Reaction
		abilitySystem->CancelAbilitiesWithTag(Tag_Hit);
		abilitySystem->TryActivateAbility(EAbility::Hit);
	}
}

void CEnemy::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);
}

Vec2 CEnemy::GetKnockbackVelocity(CGameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER * -0.6f);
}
