#include "pch.h"
#include "CEnemy.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Component/CAIController.h"
#include "Game/Component/CCharacterMovement.h"


CEnemy::CEnemy()
{
	name = TEXT("몬스터");
	pushbackForce = Vec2(0.f,0.f);
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
	// 이동 차단 태그 그룹
	const EStateTag TAG_MOVEMENT_BLOCKED = Tag_StopVelocity | Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement;
	
	// 이동 불가 상태
	if (stateSystem->HasAnyTag(TAG_MOVEMENT_BLOCKED))
	{
		StopHorizontalMovement();
		return;
	}

	// 순찰 모드
	if (stateSystem->HasTag(Tag_AIPatrol))
	{
		UpdatePatrolMovement();
	}
	// 추격 모드
	else if (stateSystem->HasTag(Tag_AIChase))
	{
		UpdateChaseMovement();
	}
	// 기본 상태 - 정지
	else
	{
		StopHorizontalMovement();
	}
}

void CEnemy::UpdatePatrolMovement()
{
	int dir = aiController->GetPatrolDirection();

	// 안전 범위 경계 체크 - 방향 전환
	if (aiController->IsAtBoundary(dir))
	{
		aiController->FlipPatrolDirection();
		dir = aiController->GetPatrolDirection();
	}

	MoveInDirection(dir, aiController->GetConfig().patrolSpeed);
}

void CEnemy::UpdateChaseMovement()
{
	int dir = aiController->GetDirectionToTarget();

	// 공격 범위 내면 정지 + 공격 시도
	if (aiController->IsTargetInAttackRange())
	{
		StopHorizontalMovement();
		abilitySystem->TryActivateAbility(EAbility::Attack);
	}
	// 안전 범위 밖이면 정지
	else if (aiController->IsAtBoundary(dir))
	{
		StopHorizontalMovement();
	}
	else
	{
		MoveInDirection(dir, aiController->GetConfig().chaseSpeed);
	}
}

void CEnemy::MoveInDirection(int dir, float speed)
{
	SetForward(dir);
	animator->SetDirection(dir);
	rigidbody->SetVelocity(Vec2(speed * dir, rigidbody->GetVelocity().y));
}

void CEnemy::OnDamage(CGameObject* source, const CombatContext& context)
{
	if (stateSystem->HasTag(Tag_Dead))
		return;

	// Trigger Event
	abilitySystem->TriggerEvent(EGameEvent::Hit, source);

	// Spawn VFX
	SpawnDamageVFX(context, source->GetForward());

	if (context.value > 0.0001f)
	{
		// Hit Reaction
		abilitySystem->CancelAbilitiesWithTag(Tag_Hit);
		abilitySystem->TryActivateAbility(EAbility::HitReact);

		// Apply Damage
		statComponent->TakeDamage(context.value);
	}
}

void CEnemy::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);
}

void CEnemy::OnDieComplete()
{
	CCharacter::OnDieComplete();
	
	SetLifetime(2.0f);
}