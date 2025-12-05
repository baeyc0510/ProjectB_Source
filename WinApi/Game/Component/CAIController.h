#pragma once
#include "Core/Component.h"
#include "Core/CGameObject.h"
#include "Game/Enum.h"

class CPlayer;
class CAbilitySystem;
class CStateSystem;

// AI 설정
struct FAIConfig
{
	float detectRange = 200.0f;		// 플레이어 감지 범위 (X축)
	float detectRangeY = 50.0f;		// 플레이어 감지 범위 (Y축) - 다른 층 무시
	float attackRange = 50.0f;		// 공격 범위
	float loseTargetRange = 300.0f;	// 타겟을 놓치는 범위
	float patrolRange = 300.0f;		// 순찰 범위 (시작점 기준)
	float patrolSpeed = 50.0f;		// 순찰 속도
	float chaseSpeed = 100.0f;		// 추격 속도
};

class CAIController : public Component<CGameObject>
{
public:
	CAIController();
	virtual ~CAIController();

	/*~ CAIController Interface ~*/
	// 설정
	void SetConfig(const FAIConfig& inConfig) { config = inConfig; }
	FAIConfig& GetConfig() { return config; }

	// 타겟
	CGameObject* GetTarget() const { return target; }
	bool HasTarget() const { return target != nullptr; }

	// 유틸리티 (Ability에서 사용)
	float GetDistanceToTarget() const;
	float GetDistanceToTargetY() const;
	int GetDirectionToTarget() const;	// -1 또는 1
	bool IsTargetInAttackRange() const;
	bool IsTargetOnSameLevel() const;	// Y축 범위 내에 있는지

	// 안전 이동 범위 (플랫폼 경계와 순찰 범위의 교집합)
	float GetSafeMinX() const;
	float GetSafeMaxX() const;
	bool IsPositionSafe(float x) const;

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
	void ComponentRender() override {}
	void ComponentRelease() override {}
	
private:
	void UpdateTargetDetection();
	void UpdatePatrol();
	void FindPlayer();
	void OnStateChanged(EStateTag oldTags, EStateTag newTags);

private:
	FAIConfig config;

	// 타겟
	CGameObject* target = nullptr;
	bool hadTargetLastFrame = false;
	bool wasInAttackRange = false;

	// 순찰
	Vec2 patrolOrigin;
	int patrolDirection = 1;

	// 캐시
	CAbilitySystem* abilitySystem = nullptr;
	CStateSystem* stateSystem = nullptr;
};
