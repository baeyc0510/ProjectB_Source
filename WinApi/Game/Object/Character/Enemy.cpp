#include "pch.h"
#include "Enemy.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/AIController.h"
#include "Game/Component/CharacterMovement.h"


Enemy::Enemy()
{
	name = TEXT("몬스터");
	pushbackForce = Vec2(0.f,0.f);
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{
	Character::Init();

	// Movement (AI 설정: 엣지 블로킹, 방향 전환)
	FMovementConfig moveConfig;
	moveConfig.maxSlopeAngle = 50.0f;
	moveConfig.bCanDropThrough = false;
	moveConfig.bBlockAtEdges = true;
	moveConfig.bFlipDirectionAtEdge = true;
	moveConfig.bFlipDirectionAtWall = true;
	movement->SetConfig(moveConfig);

	// AIController
	aiController = new AIController();
	AddChild(aiController);
}

void Enemy::OnEnable()
{
	Character::OnEnable();
	animator->Play(AnimKey::Idle);
}

void Enemy::Update()
{
	Character::Update();

	UpdateStates();
	HandleMovementEvents();
	UpdateAIMovement();
	UpdateAnimation();
}

void Enemy::Render()
{
	Character::Render();
}

void Enemy::OnDisable()
{
	Character::OnDisable();
}

void Enemy::Release()
{
	Character::Release();
}

void Enemy::UpdateAnimation()
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

void Enemy::HandleMovementEvents()
{
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

void Enemy::UpdateAIMovement()
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

void Enemy::UpdatePatrolMovement()
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

void Enemy::UpdateChaseMovement()
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

void Enemy::MoveInDirection(int dir, float speed)
{
	SetForward(dir);
	animator->SetDirection(dir);
	rigidbody->SetVelocity(Vec2(speed * dir, rigidbody->GetVelocity().y));
}

void Enemy::OnDamage(GameObject* source, const CombatContext& context)
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

void Enemy::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	Character::OnStateChanged(oldTags, newTags);
}

void Enemy::OnDieComplete()
{
	Character::OnDieComplete();
	
	SetLifetime(2.0f);
}