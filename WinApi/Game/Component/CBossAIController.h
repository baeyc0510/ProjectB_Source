#pragma once
#include "Core/Component.h"
#include "Core/CGameObject.h"
#include "Game/Enum.h"

class CPlayer;
class CStateSystem;

// 보스 공격 데이터
struct FBossAttackData
{
	EAbility ability = EAbility::None;
	float cooldown = 0.f;			// 쿨다운 시간
	float currentCooldown = 0.f;	// 현재 쿨다운
	float minRange = 0.f;			// 최소 거리
	float maxRange = 9999.f;		// 최대 거리
	float weight = 1.f;				// 선택 가중치
};

class CBossAIController : public Component<CGameObject>
{
public:
	CBossAIController();
	virtual ~CBossAIController();

	/*~ CBossAIController Interface ~*/
	// 공격 등록
	void RegisterAttack(EAbility ability, float cooldown, float minRange, float maxRange, float weight = 1.f);

	// 타겟
	CGameObject* GetTarget() const { return target; }
	bool HasTarget() const { return target != nullptr; }

	// 거리 유틸리티
	float GetDistanceToTarget() const;
	float GetDistanceToTargetY() const;
	int GetDirectionToTarget() const;	// -1 또는 1
	bool IsTargetInRange(float minDist, float maxDist) const;

	// 공격 선택 (거리 + 쿨타임 + 랜덤 가중치)
	EAbility SelectNextAttack();
	void NotifyAttackUsed(EAbility ability);
	bool IsAttackReady(EAbility ability) const;

	// 결정 타이머
	void SetDecisionInterval(float interval) { decisionInterval = interval; }

protected:
	/*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentOnEnable() override;
	void ComponentUpdate() override;
	void ComponentRender() override {}
	void ComponentOnDisable() override {}
	void ComponentRelease() override {}

private:
	void FindPlayer();
	void UpdateCooldowns(float dt);
	float GetTotalWeight(const vector<FBossAttackData*>& validAttacks) const;
	EAbility SelectByWeight(const vector<FBossAttackData*>& validAttacks, float totalWeight) const;

private:
	vector<FBossAttackData> attacks;
	CGameObject* target = nullptr;

	// 결정 타이머 (매 프레임 공격 체크 방지)
	float decisionTimer = 0.f;
	float decisionInterval = 0.5f;

	// 캐시
	CStateSystem* stateSystem = nullptr;
};
