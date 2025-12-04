#include "pch.h"
#include "CEnemy.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAIController.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Components/CLineCollider.h"

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
	if (stateSystem->HasTag(Tag_StopVelocity))
	{
		rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
		return;
	}

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

void CEnemy::UpdateAIMovement()
{
	// 이동 불가 상태면 정지
	if (stateSystem->HasAnyTag(Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement))
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
	abilitySystem->TriggerEvent(EGameEvent::Hit,source);
	
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

void CEnemy::OnCollisionStay(CCollider* other)
{
	CCharacter::OnCollisionStay(other);

	// 순찰 중 벽 충돌 감지
	if (!stateSystem->HasTag(Tag_AIPatrol))
		return;

	ELayer layer = static_cast<ELayer>(other->GetLayer());
	if (layer != ELayer::Ground)
		return;

	// 수평 충돌 판정 (벽)
	Vec2 myPos = collider->GetPos();
	Vec2 myHalf = collider->GetScale() * 0.5f;
	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;

	float overlapX = (myHalf.x + otherHalf.x) - abs(myPos.x - otherPos.x);
	float overlapY = (myHalf.y + otherHalf.y) - abs(myPos.y - otherPos.y);

	// X축 겹침이 더 작으면 벽 충돌
	if (overlapX > 0 && overlapY > 0 && overlapX < overlapY)
	{
		int wallDir = (myPos.x < otherPos.x) ? 1 : -1;
		int patrolDir = aiController->GetPatrolDirection();

		// 벽 방향으로 이동 중이면 방향 전환
		if (wallDir == patrolDir)
		{
			aiController->FlipPatrolDirection();
		}
	}
}

void CEnemy::OnCollisionExit(CCollider* other)
{
	CCharacter::OnCollisionExit(other);

	// // 순찰 또는 추격 중이 아니면 무시
	// if (!stateSystem->HasAnyTag(Tag_AIPatrol | Tag_AIChase))
	// 	return;

	ELayer layer = static_cast<ELayer>(other->GetLayer());
	if (layer != ELayer::Ground && layer != ELayer::Platform)
		return;

	// 현재 이동 방향
	int dir = stateSystem->HasTag(Tag_AIPatrol)
		? aiController->GetPatrolDirection()
		: aiController->GetDirectionToTarget();

	// 이동 방향 앞쪽 아래로 trace
	Vec2 traceCenter = GetPos();
	traceCenter.x += dir * GROUND_CHECK_AHEAD;
	// traceCenter.y += GROUND_CHECK_BELOW;
	Vec2 traceHalfSize = { GROUND_CHECK_SIZE, GROUND_CHECK_SIZE };

	// Ground와 Platform 레이어 체크
	auto groundResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Ground,true);
	auto platformResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Platform);

	bool hasGroundAhead = !groundResults.empty() || !platformResults.empty();

	if (!hasGroundAhead)
	{
		// 즉시 정지
		rigidbody->SetVelocity(Vec2(0.0f, rigidbody->GetVelocity().y));

		// Exit된 콜라이더의 경계 구하기
		float edgeMinX, edgeMaxX;
		CLineCollider* lineCollider = dynamic_cast<CLineCollider*>(other);
		if (lineCollider)
		{
			Vec2 start = lineCollider->GetWorldStart();
			Vec2 end = lineCollider->GetWorldEnd();
			edgeMinX = min(start.x, end.x);
			edgeMaxX = max(start.x, end.x);
		}
		else
		{
			Vec2 otherPos = other->GetPos();
			float otherHalfX = other->GetScale().x * 0.5f;
			edgeMinX = otherPos.x - otherHalfX;
			edgeMaxX = otherPos.x + otherHalfX;
		}

		// 안전한 위치로 보정 (플랫폼 경계 안쪽으로)
		Vec2 safePos = GetPos();
		float myHalfWidth = collider->GetScale().x * 0.5f;
		Vec2 offset = collider->GetOffset();
		constexpr float EDGE_MARGIN = 5.0f;

		if (dir > 0)  // 오른쪽 가장자리에서 Exit
		{
			float safeX = edgeMaxX - myHalfWidth - EDGE_MARGIN;
			safePos.x = safeX - offset.x;
		}
		else  // 왼쪽 가장자리에서 Exit
		{
			float safeX = edgeMinX + myHalfWidth + EDGE_MARGIN;
			safePos.x = safeX - offset.x;
		}
		SetPos(safePos);

		// 순찰 중이면 방향 전환
		if (stateSystem->HasTag(Tag_AIPatrol))
		{
			aiController->FlipPatrolDirection();
		}
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
