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

private:
	void SpawnVineHazard(Vec2 pos);
};
