#include "pch.h"
#include "CAIController.h"
#include "CAbilitySystem.h"
#include "CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Object/Character/CCharacter.h"

CAIController::CAIController()
{
}

CAIController::~CAIController()
{
}

void CAIController::ComponentInit()
{
	// 컴포넌트 캐시
	abilitySystem = owner->GetComponent<CAbilitySystem>();
	stateSystem = owner->GetComponent<CStateSystem>();

	// 상태 변경 이벤트 구독
	if (stateSystem)
	{
		stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags) {
			OnStateChanged(oldTags, newTags);
		});
	}

	// 순찰 시작점 설정
	patrolOrigin = owner->GetPos();
}

void CAIController::ComponentOnEnable()
{
	// 초기 상태: 순찰
	if (stateSystem)
	{
		stateSystem->AddTagUnique(Tag_AIPatrol);
	}
}

void CAIController::ComponentUpdate()
{
	// 피격/스턴/공격 중에는 AI 업데이트 중지
	if (stateSystem && stateSystem->HasAnyTag(Tag_Hit | Tag_Stunned | Tag_Attacking))
		return;

	UpdateTargetDetection();
	UpdatePatrol();
}

void CAIController::UpdateTargetDetection()
{
	// 플레이어 찾기
	if (!target)
	{
		FindPlayer();
	}

	// 타겟이 없으면 리턴
	if (!target)
	{
		if (hadTargetLastFrame && stateSystem)
		{
			stateSystem->RemoveTag(Tag_HasTarget);
		}
		hadTargetLastFrame = false;
		return;
	}

	float distance = GetDistanceToTarget();
	bool isOnSameLevel = IsTargetOnSameLevel();
	bool isInDetectRange = distance <= config.detectRange && isOnSameLevel;
	bool isInAttackRange = distance <= config.attackRange && isOnSameLevel;
	bool isOutOfRange = distance > config.loseTargetRange || !isOnSameLevel;

	// 범위 이탈 - 타겟 해제
	if (isOutOfRange)
	{
		target = nullptr;
		if (stateSystem)
		{
			stateSystem->RemoveTag(Tag_HasTarget);
		}
		if (abilitySystem)
		{
			abilitySystem->TriggerEvent(EGameEvent::AI_TargetLost, nullptr);
		}
		hadTargetLastFrame = false;
		wasInAttackRange = false;
		return;
	}

	// 최초 감지
	if (!hadTargetLastFrame && isInDetectRange)
	{
		if (stateSystem)
		{
			stateSystem->AddTagUnique(Tag_HasTarget);
		}
		if (abilitySystem)
		{
			abilitySystem->TriggerEvent(EGameEvent::AI_TargetDetected, target);
		}
		hadTargetLastFrame = true;
	}

	// 공격 범위 진입
	if (!wasInAttackRange && isInAttackRange)
	{
		if (abilitySystem)
		{
			abilitySystem->TriggerEvent(EGameEvent::AI_TargetInAttackRange, target);
		}
		wasInAttackRange = true;
	}
	else if (wasInAttackRange && !isInAttackRange)
	{
		wasInAttackRange = false;
	}
}

void CAIController::FindPlayer()
{
	if (!owner || !owner->GetScene())
		return;

	target = owner->GetScene()->FindObjectByType<CPlayer>();
}

float CAIController::GetDistanceToTarget() const
{
	if (!target || !owner)
		return FLT_MAX;

	Vec2 ownerPos = owner->GetPos();
	Vec2 targetPos = target->GetPos();
	Vec2 diff = targetPos - ownerPos;
	return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

int CAIController::GetDirectionToTarget() const
{
	if (!target || !owner)
		return 1;

	float diff = target->GetPos().x - owner->GetPos().x;
	return diff >= 0 ? 1 : -1;
}

bool CAIController::IsTargetInAttackRange() const
{
	if (GetDistanceToTarget() > config.attackRange)
		return false;

	// 타겟이 앞에 있어야 하는 경우 방향 체크
	if (config.requireFacingTarget && owner)
	{
		int facingDir = (owner->GetScale().x >= 0) ? 1 : -1;
		int targetDir = GetDirectionToTarget();
		if (facingDir != targetDir)
			return false;
	}

	return true;
}

bool CAIController::IsAtPatrolBoundary() const
{
	if (!owner)
		return false;

	float currentX = owner->GetPos().x;
	float minX = patrolOrigin.x - config.patrolRange;
	float maxX = patrolOrigin.x + config.patrolRange;

	return currentX <= minX || currentX >= maxX;
}

void CAIController::UpdatePatrol()
{
	// 순찰 중이 아니면 리턴
	if (!stateSystem || !stateSystem->HasTag(Tag_AIPatrol))
		return;

	// 순찰 경계 도달 시 방향 전환 + 이벤트 발생
	if (IsAtPatrolBoundary())
	{
		FlipPatrolDirection();
		if (abilitySystem)
		{
			abilitySystem->TriggerEvent(EGameEvent::AI_PatrolPointReached, nullptr);
		}
	}
}

void CAIController::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	// Tag_HasTarget 추가 : 추격 모드
	if (TagAdded(oldTags, newTags, Tag_HasTarget))
	{
		stateSystem->RemoveTag(Tag_AIPatrol);
		stateSystem->AddTagUnique(Tag_AIChase);
	}
	// Tag_HasTarget 제거 : 순찰 모드
	else if (TagRemoved(oldTags, newTags, Tag_HasTarget))
	{
		stateSystem->RemoveTag(Tag_AIChase);
		stateSystem->AddTagUnique(Tag_AIPatrol);
		// 순찰 시작점 갱신
		patrolOrigin = owner->GetPos();
	}
}

float CAIController::GetDistanceToTargetY() const
{
	if (!target || !owner)
		return FLT_MAX;

	return abs(target->GetPos().y - owner->GetPos().y);
}

bool CAIController::IsTargetOnSameLevel() const
{
	return GetDistanceToTargetY() <= config.detectRangeY;
}

float CAIController::GetSafeMinX() const
{
	return patrolOrigin.x - config.patrolRange;
}

float CAIController::GetSafeMaxX() const
{
	return patrolOrigin.x + config.patrolRange;
}

bool CAIController::IsPositionSafe(float x) const
{
	return x >= GetSafeMinX() && x <= GetSafeMaxX();
}
