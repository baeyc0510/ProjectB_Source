#include "pch.h"
#include "CBossAIController.h"
#include "CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Object/Character/CCharacter.h"

CBossAIController::CBossAIController()
{
}

CBossAIController::~CBossAIController()
{
}

void CBossAIController::ComponentInit()
{
	stateSystem = owner->GetComponent<CStateSystem>();
}

void CBossAIController::ComponentOnEnable()
{
	FindPlayer();
}

void CBossAIController::ComponentUpdate()
{
	// 피격/스턴/공격 중에는 AI 업데이트 중지
	if (stateSystem && stateSystem->HasAnyTag(Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BossAppearing))
		return;

	// 플레이어 찾기
	if (!target)
	{
		FindPlayer();
	}

	// 쿨다운 업데이트
	UpdateCooldowns(DT);

	// 결정 타이머 업데이트
	decisionTimer += DT;
}

void CBossAIController::FindPlayer()
{
	if (!owner || !owner->GetScene())
		return;

	target = owner->GetScene()->FindObjectByType<CPlayer>();
}

void CBossAIController::UpdateCooldowns(float dt)
{
	for (auto& attack : attacks)
	{
		if (attack.currentCooldown > 0.f)
		{
			attack.currentCooldown -= dt;
		}
	}
}

void CBossAIController::RegisterAttack(EAbility ability, float cooldown, float minRange, float maxRange, float weight)
{
	FBossAttackData data;
	data.ability = ability;
	data.cooldown = cooldown;
	data.currentCooldown = 0.f;
	data.minRange = minRange;
	data.maxRange = maxRange;
	data.weight = weight;
	attacks.push_back(data);
}

float CBossAIController::GetDistanceToTarget() const
{
	if (!target || !owner)
		return FLT_MAX;

	Vec2 ownerPos = owner->GetPos();
	Vec2 targetPos = target->GetPos();
	return abs(targetPos.x - ownerPos.x);  // X축 거리만 사용 (보스는 보통 같은 층에서 싸움)
}

float CBossAIController::GetDistanceToTargetY() const
{
	if (!target || !owner)
		return FLT_MAX;

	return abs(target->GetPos().y - owner->GetPos().y);
}

int CBossAIController::GetDirectionToTarget() const
{
	if (!target || !owner)
		return 1;

	float diff = target->GetPos().x - owner->GetPos().x;
	return diff >= 0 ? 1 : -1;
}

bool CBossAIController::IsTargetInRange(float minDist, float maxDist) const
{
	float dist = GetDistanceToTarget();
	return dist >= minDist && dist <= maxDist;
}

EAbility CBossAIController::SelectNextAttack()
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
		// 쿨다운 체크
		if (attack.currentCooldown > 0.f)
			continue;

		// 거리 체크
		if (distance < attack.minRange || distance > attack.maxRange)
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

float CBossAIController::GetTotalWeight(const vector<FBossAttackData*>& validAttacks) const
{
	float total = 0.f;
	for (const auto* attack : validAttacks)
	{
		total += attack->weight;
	}
	return total;
}

EAbility CBossAIController::SelectByWeight(const vector<FBossAttackData*>& validAttacks, float totalWeight) const
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

void CBossAIController::NotifyAttackUsed(EAbility ability)
{
	for (auto& attack : attacks)
	{
		if (attack.ability == ability)
		{
			attack.currentCooldown = attack.cooldown;
			break;
		}
	}
}

bool CBossAIController::IsAttackReady(EAbility ability) const
{
	for (const auto& attack : attacks)
	{
		if (attack.ability == ability)
		{
			return attack.currentCooldown <= 0.f;
		}
	}
	return false;
}
