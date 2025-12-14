#include "pch.h"
#include "StatComponent.h"
#include "AbilitySystem.h"
#include "StateSystem.h"

StatComponent::StatComponent()
{
}

StatComponent::~StatComponent()
{
}

void StatComponent::ComponentInit()
{
	// 컴포넌트 캐싱
	abilitySystem = GetOwner()->GetComponent<AbilitySystem>();
	stateSystem = GetOwner()->GetComponent<StateSystem>();
}

// 스탯 초기화
void StatComponent::InitStat(EStatType type, float maxValue)
{
	InitStat(type, maxValue, maxValue);
}

void StatComponent::InitStat(EStatType type, float currentValue, float maxValue)
{
	FStat& stat = stats[type];
	stat.max = max(0.f, maxValue);
	stat.current = max(0.f, min(currentValue, stat.max));

	OnStatChanged.Broadcast(type, stat.current, stat.max);

	// Flask 태그 업데이트
	if (type == EStatType::Flask)
	{
		UpdateFlaskTag();
	}
}

// 스탯 설정
void StatComponent::SetCurrent(EStatType type, float value)
{
	auto it = stats.find(type);
	if (it == stats.end())
		return;

	FStat& stat = it->second;
	float oldValue = stat.current;
	stat.current = max(0.f, min(value, stat.max));

	if (!IsNearlyEqual(oldValue, stat.current))
	{
		OnStatChanged.Broadcast(type, stat.current, stat.max);

		// HP 특수 처리
		if (type == EStatType::HP)
		{
			HandleHPChanged(oldValue, stat.current);
		}
		// Flask 태그 업데이트
		else if (type == EStatType::Flask)
		{
			UpdateFlaskTag();
		}
	}
}

void StatComponent::SetMax(EStatType type, float value)
{
	auto it = stats.find(type);
	if (it == stats.end())
		return;

	FStat& stat = it->second;
	float oldMax = stat.max;
	stat.max = max(0.f, value);

	// current가 max를 초과하면 조정
	if (stat.current > stat.max)
	{
		stat.current = stat.max;
	}

	if (!IsNearlyEqual(oldMax, stat.max))
	{
		OnStatChanged.Broadcast(type, stat.current, stat.max);
	}
}

// 스탯 변경 (델타)
void StatComponent::ModifyCurrent(EStatType type, float delta)
{
	float current = GetCurrent(type);
	SetCurrent(type, current + delta);
}

// 스탯 조회
float StatComponent::GetCurrent(EStatType type) const
{
	auto it = stats.find(type);
	if (it != stats.end())
		return it->second.current;
	return 0.f;
}

float StatComponent::GetMax(EStatType type) const
{
	auto it = stats.find(type);
	if (it != stats.end())
		return it->second.max;
	return 0.f;
}

bool StatComponent::HasStat(EStatType type) const
{
	return stats.find(type) != stats.end();
}

// HP 편의 함수
bool StatComponent::IsAlive() const
{
	return GetCurrent(EStatType::HP) > 0.f;
}

void StatComponent::TakeDamage(float damage)
{
	ModifyCurrent(EStatType::HP, -damage);
}

void StatComponent::Heal(float amount)
{
	ModifyCurrent(EStatType::HP, amount);
}

// 내부 함수
void StatComponent::HandleHPChanged(float oldHP, float newHP)
{
	// HP가 0이 되면 사망 처리
	if (IsNearlyEqual(newHP, 0.f) && !IsNearlyEqual(oldHP, 0.f))
	{
		if (abilitySystem)
		{
			abilitySystem->TryActivateAbility(EAbility::Die);
		}
	}
	// HP가 0에서 회복되면 (부활) Die 캔슬
	else if (!IsNearlyEqual(newHP, 0.f) && IsNearlyEqual(oldHP, 0.f))
	{
		if (abilitySystem && stateSystem && stateSystem->HasTag(Tag_Dead))
		{
			abilitySystem->CancelAbilitiesWithTag(Tag_Dead);
		}
	}
}

void StatComponent::UpdateFlaskTag()
{
	if (!stateSystem)
		return;

	float flaskCount = GetCurrent(EStatType::Flask);
	if (flaskCount > 0.f)
	{
		stateSystem->AddTagUnique(Tag_FlaskRemaining);
	}
	else
	{
		stateSystem->RemoveTag(Tag_FlaskRemaining);
	}
}
