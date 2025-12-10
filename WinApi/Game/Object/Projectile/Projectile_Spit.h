#pragma once
#include "Projectile.h"

class Projectile_Spit : public Projectile
{
public:
	Projectile_Spit();

protected:
	void Init() override;
	void OnHitGround(Vec2 hitPos) override;
	void OnHitWall(Vec2 hitPos) override;
	void OnHitPlayer(CGameObject* player) override;
	void OnDisable() override;
	void Release() override;
	
	void OnDamage(CGameObject* source, const CombatContext& context) override;
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
