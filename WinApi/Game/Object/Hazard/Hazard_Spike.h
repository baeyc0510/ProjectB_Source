#pragma once
#include "Hazard.h"
#include "Game/Util/CombatHelper.h"

enum class EGameEvent;

class Hazard_Spike : public Hazard
{
public:
	Hazard_Spike();

	// 지연 스폰 설정 (순차적 스파이크 생성용)
	void SetSpawnDelay(float delay) { spawnDelay = delay; }

protected:
	/*~ GameObject Interface ~*/
	void Init() override;
	void Update() override;
	void OnSpawn() override;
	void OnCollisionStay(Collider* other) override;
	
	/*~ Hazard_Spike Interface ~*/
	void OnExpire();
	void OnGameEvent(EGameEvent event);
	
private:
	static const AttackData AttackData;
	
	float spawnDelay = 0.f;
	float currentDelay = 0.f;
	bool bHasSpawned = false;
	bool bDamageEnabled = false;
};
