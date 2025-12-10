#pragma once
#include "Game/Enum.h"

// 스탯 타입 열거형
enum class EStatType
{
	HP,
	MP,
	Flask,
	JumpForce,
	AttackPower,
	// 필요시 확장
};

// 단일 스탯 데이터
struct FStat
{
	float current = 0.f;
	float max = 0.f;
};

class AbilitySystem;
class StateSystem;

class StatComponent : public Component<CGameObject>
{
public:
	StatComponent();
	virtual ~StatComponent();

	/*~ Stat Management ~*/
	// 스탯 초기화
	void InitStat(EStatType type, float maxValue);
	void InitStat(EStatType type, float currentValue, float maxValue);

	// 스탯 설정
	void SetCurrent(EStatType type, float value);
	void SetMax(EStatType type, float value);

	// 스탯 변경 (델타)
	void ModifyCurrent(EStatType type, float delta);

	// 스탯 조회
	float GetCurrent(EStatType type) const;
	float GetMax(EStatType type) const;
	bool HasStat(EStatType type) const;

	// HP 편의 함수
	bool IsAlive() const;
	void TakeDamage(float damage);
	void Heal(float amount);

	/*~ Events ~*/
	// 스탯 변경 이벤트 (type, newCurrent, newMax)
	MulticastDelegate<EStatType, float, float> OnStatChanged;

protected:
	/*~ Component Interface ~*/
	void ComponentInit() override;

private:
	void HandleHPChanged(float oldHP, float newHP);
	void UpdateFlaskTag();

private:
	map<EStatType, FStat> stats;

	// 캐시된 컴포넌트 참조
	AbilitySystem* abilitySystem = nullptr;
	StateSystem* stateSystem = nullptr;
};
