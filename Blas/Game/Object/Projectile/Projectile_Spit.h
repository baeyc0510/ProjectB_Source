#pragma once
#include "Projectile.h"

class Projectile_Spit : public Projectile
{
public:
	Projectile_Spit();

protected:
	void Init() override;
	void OnHitGround(Vec2 hitPos, Collider* collider) override;
	void OnHitWall(Vec2 hitPos) override;
	void OnHitPlayer(GameObject* player) override;
	void OnDisable() override;
	void Release() override;
	
	void OnDamage(GameObject* source, const CombatContext& context) override;
private:
	void SpawnSpike(Vec2 spawnPos);
	
private:
	bool bIsThorn = false;
	bool bGrowthFinished;
	bool bSpikeSpawned = false;
	bool bIsDestroyed = false;
	Vec2 groundHitPos;
	
	SafeTimerHandle timerHandle;
};
