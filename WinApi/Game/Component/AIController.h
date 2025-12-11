#pragma once
#include "AIControllerBase.h"
#include "Game/Enum.h"

// AI 설정
struct AIConfig
{
	float detectRange = 200.0f;		// 플레이어 감지 범위 (X축)
	float detectRangeY = 50.0f;		// 플레이어 감지 범위 (Y축) - 다른 층 무시
	float attackRange = 50.0f;		// 공격 범위
	float loseTargetRange = 300.0f;	// 타겟을 놓치는 범위
	float patrolRange = 300.0f;		// 순찰 범위 (시작점 기준)
	float patrolSpeed = 50.0f;		// 순찰 속도
	float chaseSpeed = 100.0f;		// 추격 속도
	float detectionInterval = 0.1f;	// 감지 주기 (초)
	bool requireFacingTarget = true;	// true면 공격 범위 판정 시 타겟이 앞에 있어야 함
};

class AIController : public AIControllerBase
{
public:
	AIController();
	virtual ~AIController();

	/*~ AIController Interface ~*/
	// 설정
	void SetConfig(const AIConfig& inConfig) { config = inConfig; }
	AIConfig& GetConfig() { return config; }

	// 유틸리티 (Ability에서 사용)
	bool IsTargetInAttackRange() const;
	bool IsTargetOnSameLevel() const;	// Y축 범위 내에 있는지

	// 안전 이동 범위 (플랫폼 경계와 순찰 범위의 교집합)
	float GetSafeMinX() const;
	float GetSafeMaxX() const;
	bool IsPositionSafe(float x) const;
	bool IsAtBoundary(int dir) const;		// 해당 방향으로 이동 시 경계에 도달했는지

	// 순찰
	Vec2 GetPatrolOrigin() const { return patrolOrigin; }
	void SetPatrolOrigin(Vec2 origin) { patrolOrigin = origin; }
	int GetPatrolDirection() const { return patrolDirection; }
	void SetPatrolDirection(int dir) { patrolDirection = dir; }
	void FlipPatrolDirection() { patrolDirection *= -1; }
	bool IsAtPatrolBoundary() const;

protected:
	/*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentUpdate() override;
	void ComponentOnEnable() override;

private:
	void UpdateTargetDetection();
	void UpdatePatrol();
	void OnStateChanged(EStateTag oldTags, EStateTag newTags);

	// 조건부 업데이트 (성능 최적화)
	bool ShouldUpdate() const;
	bool IsOnScreen() const;

	// 업데이트 최적화 상수
	static constexpr float MAX_UPDATE_DISTANCE = 500.f;
	static constexpr float SCREEN_MARGIN = 50.f;

private:
	AIConfig config;

	// Components
	StateSystem* stateSystem = nullptr;
	AbilitySystem* abilitySystem = nullptr;
	
	// 타겟 감지 상태
	float detectionTimer = 0.f;
	bool hadTargetLastFrame = false;
	bool wasInAttackRange = false;

	// 순찰
	Vec2 patrolOrigin;
	int patrolDirection = 1;

	// 이벤트 구독 (RAII 자동 해제)
	SafeDelegateHandle<EStateTag, EStateTag> stateChangedHandle;
};
