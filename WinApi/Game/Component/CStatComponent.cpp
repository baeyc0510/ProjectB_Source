#include "pch.h"
#include "CStatComponent.h"
#include "CAbilitySystem.h"
#include "CStateSystem.h"

CStatComponent::CStatComponent()
{
}

CStatComponent::~CStatComponent()
{
}

void CStatComponent::ComponentInit()
{
	// 컴포넌트 캐싱
	abilitySystem = GetOwner()->GetComponent<CAbilitySystem>();
	stateSystem = GetOwner()->GetComponent<CStateSystem>();
}

// 스탯 초기화
void CStatComponent::InitStat(EStatType type, float maxValue)
{
	InitStat(type, maxValue, maxValue);
}

void CStatComponent::InitStat(EStatType type, float currentValue, float maxValue)
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
void CStatComponent::SetCurrent(EStatType type, float value)
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

void CStatComponent::SetMax(EStatType type, float value)
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
void CStatComponent::ModifyCurrent(EStatType type, float delta)
{
	float current = GetCurrent(type);
	SetCurrent(type, current + delta);
}

// 스탯 조회
float CStatComponent::GetCurrent(EStatType type) const
{
	auto it = stats.find(type);
	if (it != stats.end())
		return it->second.current;
	return 0.f;
}

float CStatComponent::GetMax(EStatType type) const
{
	auto it = stats.find(type);
	if (it != stats.end())
		return it->second.max;
	return 0.f;
}

bool CStatComponent::HasStat(EStatType type) const
{
	return stats.find(type) != stats.end();
}

// HP 편의 함수
bool CStatComponent::IsAlive() const
{
	return GetCurrent(EStatType::HP) > 0.f;
}

void CStatComponent::TakeDamage(float damage)
{
	ModifyCurrent(EStatType::HP, -damage);
}

void CStatComponent::Heal(float amount)
{
	ModifyCurrent(EStatType::HP, amount);
}

// 내부 함수
void CStatComponent::HandleHPChanged(float oldHP, float newHP)
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

void CStatComponent::UpdateFlaskTag()
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
