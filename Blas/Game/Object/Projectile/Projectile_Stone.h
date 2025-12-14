#pragma once
#include "Projectile.h"

class Projectile_Stone : public Projectile
{
public:
    
protected:
    /*~ Projectile Interface ~*/
    void Init() override;
    void Update() override;
    void OnHitPlayer(GameObject* player) override;
    void OnHitGround(Vec2 hitPos, Collider* groundCol) override;
};
