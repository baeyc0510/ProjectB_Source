#include "pch.h"
#include "BossAIController.h"
#include "StateSystem.h"
#include <ctime>

BossAIController::BossAIController()
{
	// 랜덤 시드 설정 (한 번만)
	static bool bSeeded = false;
	if (!bSeeded)
	{
		srand(static_cast<unsigned int>(time(nullptr)));
		bSeeded = true;
	}
}

BossAIController::~BossAIController()
{
}

void BossAIController::ComponentInit()
{
	// 베이스 클래스의 컴포넌트 캐싱 호출
	CacheComponents();
}

void BossAIController::ComponentOnEnable()
{
	FindPlayer();
}

void BossAIController::ComponentUpdate()
{
	// 보스 AI 업데이트 차단 태그 그룹
	const EStateTag TAG_BOSS_AI_BLOCKED = Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BossAppearing;

	// 피격/스턴/공격 중에는 AI 업데이트 중지
	if (stateSystem->HasAnyTag(TAG_BOSS_AI_BLOCKED))
		return;

	// 플레이어 찾기
	if (!target)
	{
		FindPlayer();
	}
	
	// 결정 타이머 업데이트
	decisionTimer += DT;
}

void BossAIController::RegisterAttack(EAbility ability, float minRange, float maxRange, float weight)
{
	FBossAttackData data;
	data.ability = ability;
	data.minRange = minRange;
	data.maxRange = maxRange;
	data.weight = weight;
	attacks.push_back(data);
}

float BossAIController::GetDistanceToTarget() const
{
	if (!target || !owner)
		return FLT_MAX;

	Vec2 ownerPos = owner->GetPos();
	Vec2 targetPos = target->GetPos();
	return abs(targetPos.x - ownerPos.x);  // X축 거리만 사용
}

bool BossAIController::IsTargetInRange(float minDist, float maxDist) const
{
	float dist = GetDistanceToTarget();
	return dist >= minDist && dist <= maxDist;
}

EAbility BossAIController::SelectNextAttack()
{
	// 결정 간격 체크
	if (decisionTimer < decisionInterval)
		return EAbility::None;

	// 타겟이 없으면 공격 안함
	if (!target)
		return EAbility::None;

	float distance = GetDistanceToTarget();

	// 사용 가능한 공격 목록 수집
	vector<FBossAttackData*> validAttacks;
	for (auto& attack : attacks)
	{
		// 거리 체크
		if (distance < attack.minRange || distance > attack.maxRange)
			continue;
		
		// 발동 가능 체크
		if (!abilitySystem->CanActivateAbility(attack.ability))
			continue;

		validAttacks.push_back(&attack);
	}

	// 사용 가능한 공격이 없으면 리턴
	if (validAttacks.empty())
		return EAbility::None;

	// 가중치 기반 랜덤 선택
	float totalWeight = GetTotalWeight(validAttacks);
	EAbility selected = SelectByWeight(validAttacks, totalWeight);

	// 결정 타이머 리셋
	decisionTimer = 0.f;

	return selected;
}

float BossAIController::GetTotalWeight(const vector<FBossAttackData*>& validAttacks) const
{
	float total = 0.f;
	for (const auto* attack : validAttacks)
	{
		total += attack->weight;
	}
	return total;
}

EAbility BossAIController::SelectByWeight(const vector<FBossAttackData*>& validAttacks, float totalWeight) const
{
	float random = static_cast<float>(rand()) / RAND_MAX * totalWeight;
	float accumulated = 0.f;

	for (const auto* attack : validAttacks)
	{
		accumulated += attack->weight;
		if (random <= accumulated)
		{
			return attack->ability;
		}
	}

	// 기본값 (마지막 공격 반환)
	return validAttacks.back()->ability;
}

bool BossAIController::ShouldChase() const
{
	if (!chaseConfig.bCanChase || !target)
		return false;

	float distance = GetDistanceToTarget();

	// 정지 범위 안이면 추격 불필요
	if (distance <= chaseConfig.stopRange)
		return false;

	// 추격 범위 안이면 추격
	return distance <= chaseConfig.chaseRange;
}

bool BossAIController::IsInStopRange() const
{
	if (!target)
		return false;

	return GetDistanceToTarget() <= chaseConfig.stopRange;
}