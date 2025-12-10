#pragma once
#include "AIControllerBase.h"
#include "AbilitySystem.h"
#include "Game/Enum.h"

// 보스 공격 데이터
struct FBossAttackData
{
	EAbility ability = EAbility::None;
	float minRange = 0.f;			// 최소 거리
	float maxRange = 9999.f;		// 최대 거리
	float weight = 1.f;				// 선택 가중치
};

// 보스 추격 설정
struct FBossChaseConfig
{
	float chaseSpeed = 80.0f;		// 추격 속도
	float chaseRange = 400.0f;		// 추격 시작 거리 (이 거리 이내면 추격)
	float stopRange = 60.0f;		// 정지 거리 (근접 공격 범위)
	bool bCanChase = true;			// 추격 가능 여부
};

class BossAIController : public AIControllerBase
{
public:
	BossAIController();
	virtual ~BossAIController();

	/*~ BossAIController Interface ~*/
	// 공격 등록
	void RegisterAttack(EAbility ability, float minRange, float maxRange, float weight = 1.f);

	// 거리 유틸리티 (보스는 X축 거리만 사용)
	float GetDistanceToTarget() const override;
	bool IsTargetInRange(float minDist, float maxDist) const;

	// 공격 선택 (거리 + 쿨타임 + 랜덤 가중치)
	EAbility SelectNextAttack();

	// 결정 타이머
	void SetDecisionInterval(float interval) { decisionInterval = interval; }

	// 추격 시스템
	void SetChaseConfig(const FBossChaseConfig& config) { chaseConfig = config; }
	FBossChaseConfig& GetChaseConfig() { return chaseConfig; }
	bool ShouldChase() const;			// 추격해야 하는지 (공격 범위 밖, 추격 범위 안)
	bool IsInStopRange() const;			// 정지 범위 내인지
	float GetChaseSpeed() const { return chaseConfig.chaseSpeed; }

protected:
	/*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentOnEnable() override;
	void ComponentUpdate() override;
	void ComponentRender() override {}
	void ComponentOnDisable() override {}
	void ComponentRelease() override {}

private:
	float GetTotalWeight(const vector<FBossAttackData*>& validAttacks) const;
	EAbility SelectByWeight(const vector<FBossAttackData*>& validAttacks, float totalWeight) const;

private:
	vector<FBossAttackData> attacks;

	// 결정 타이머 (매 프레임 공격 체크 방지)
	float decisionTimer = 0.f;
	float decisionInterval = 0.5f;

	// 추격 설정
	FBossChaseConfig chaseConfig;
};
