#pragma once
#include "CProjectile.h"

class CProjectile_Spit : public CProjectile
{
public:
	CProjectile_Spit();

protected:
	void Init() override;
	void OnHitGround(Vec2 hitPos) override;
	void OnHitWall(Vec2 hitPos) override;
	void OnDestroyed() override;
	void OnHitPlayer(CGameObject* player) override;
	
	void OnDamage(CGameObject* source, const CombatContext& context) override;
private:
	void SpawnSpike(Vec2 spawnPos);
	
private:
	bool bIsThorn = false;
	bool bGrowthFinished;
	bool bSpikeSpawned = false;
	Vec2 groundHitPos;
};
