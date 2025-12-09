#pragma once
#include "CHazard.h"
#include "Game/Util/CombatHelper.h"

enum class EGameEvent;

class CHazard_Spike : public CHazard
{
public:
	CHazard_Spike();

	// 지연 스폰 설정 (순차적 스파이크 생성용)
	void SetSpawnDelay(float delay) { spawnDelay = delay; }

protected:
	void Init() override;
	void Update() override;
	void OnSpawn() override;
	void OnExpire() override;
	
	void OnGameEvent(EGameEvent event);
	
private:
	void OnHitCheck();

private:
	static const AttackData AttackData;
	
	float spawnDelay = 0.f;
	float currentDelay = 0.f;
	bool bHasSpawned = false;
};
