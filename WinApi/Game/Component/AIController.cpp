#include "pch.h"
#include "AIController.h"

#include "AbilitySystem.h"
#include "StateSystem.h"

namespace
{
	// AI 업데이트 차단 태그 그룹
	const EStateTag TAG_AI_BLOCKED = Tag_Hit | Tag_Stunned | Tag_Attacking;
}

AIController::AIController()
{
}

AIController::~AIController()
{
}

void AIController::ComponentInit()
{
	// 베이스 클래스의 컴포넌트 캐싱 호출
	CacheComponents();

	// 상태 변경 이벤트 구독
	stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags) {
		OnStateChanged(oldTags, newTags);
	});

	// 순찰 시작점 설정
	patrolOrigin = owner->GetPos();
}

void AIController::ComponentOnEnable()
{
	// 초기 상태: 순찰
	stateSystem->AddTagUnique(Tag_AIPatrol);
}

void AIController::ComponentUpdate()
{
	// 업데이트 조건 체크 (화면 밖이고 멀면 스킵)
	if (!ShouldUpdate())
		return;

	// 피격/스턴/공격 중에는 AI 업데이트 중지
	if (stateSystem->HasAnyTag(TAG_AI_BLOCKED))
		return;

	// 감지는 주기적으로만 (매 프레임 연산 방지)
	detectionTimer += DT;
	if (detectionTimer >= config.detectionInterval)
	{
		detectionTimer = 0.f;
		UpdateTargetDetection();
	}

	// 순찰/추격은 매 프레임 (이동 필요)
	UpdatePatrol();
}

void AIController::UpdateTargetDetection()
{
	// 플레이어 찾기
	if (!target)
	{
		FindPlayer();
	}

	// 타겟이 없으면 리턴
	if (!target)
	{
		if (hadTargetLastFrame)
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
		stateSystem->RemoveTag(Tag_HasTarget);
		abilitySystem->TriggerEvent(EGameEvent::AI_TargetLost, nullptr);
		hadTargetLastFrame = false;
		wasInAttackRange = false;
		return;
	}

	// 최초 감지
	if (!hadTargetLastFrame && isInDetectRange)
	{
		stateSystem->AddTagUnique(Tag_HasTarget);
		abilitySystem->TriggerEvent(EGameEvent::AI_TargetDetected, target);
		hadTargetLastFrame = true;
	}

	// 공격 범위 진입
	if (!wasInAttackRange && isInAttackRange)
	{
		abilitySystem->TriggerEvent(EGameEvent::AI_TargetInAttackRange, target);
		wasInAttackRange = true;
	}
	else if (wasInAttackRange && !isInAttackRange)
	{
		wasInAttackRange = false;
	}
}


bool AIController::IsTargetInAttackRange() const
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

bool AIController::IsAtPatrolBoundary() const
{
	if (!owner)
		return false;

	float currentX = owner->GetPos().x;
	float minX = patrolOrigin.x - config.patrolRange;
	float maxX = patrolOrigin.x + config.patrolRange;

	return currentX <= minX || currentX >= maxX;
}

void AIController::UpdatePatrol()
{
	// 순찰 중이 아니면 리턴
	if (!stateSystem->HasTag(Tag_AIPatrol))
		return;

	// 순찰 경계 도달 시 방향 전환 + 이벤트 발생
	if (IsAtPatrolBoundary())
	{
		FlipPatrolDirection();
		abilitySystem->TriggerEvent(EGameEvent::AI_PatrolPointReached, nullptr);
	}
}

void AIController::OnStateChanged(EStateTag oldTags, EStateTag newTags)
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

bool AIController::IsTargetOnSameLevel() const
{
	return GetDistanceToTargetY() <= config.detectRangeY;
}

float AIController::GetSafeMinX() const
{
	return patrolOrigin.x - config.patrolRange;
}

float AIController::GetSafeMaxX() const
{
	return patrolOrigin.x + config.patrolRange;
}

bool AIController::IsPositionSafe(float x) const
{
	return x >= GetSafeMinX() && x <= GetSafeMaxX();
}

bool AIController::IsAtBoundary(int dir) const
{
	if (!owner)
		return false;

	float currentX = owner->GetPos().x;
	return (dir > 0 && currentX >= GetSafeMaxX()) || (dir < 0 && currentX <= GetSafeMinX());
}

bool AIController::ShouldUpdate() const
{
	// 타겟이 없으면 항상 업데이트 (탐색 필요)
	if (!target)
		return true;

	// 화면 내 또는 가까운 거리면 업데이트
	return IsOnScreen() || GetDistanceToTarget() <= MAX_UPDATE_DISTANCE;
}

bool AIController::IsOnScreen() const
{
	if (!owner)
		return false;

	Vec2 virtualSize = SINGLE(EngineInstance)->GetVirtualSize();
	Vec2 halfScreen = virtualSize * 0.5f;

	Vec2 ownerPos = owner->GetPos();
	Vec2 cameraPos = CAMERA->GetLookAt();

	float dx = abs(ownerPos.x - cameraPos.x);
	float dy = abs(ownerPos.y - cameraPos.y);

	return dx <= halfScreen.x + SCREEN_MARGIN
		&& dy <= halfScreen.y + SCREEN_MARGIN;
}
