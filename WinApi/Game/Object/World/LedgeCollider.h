#pragma once
#include "WorldCollider.h"

class LedgeCollider : public WorldCollider
{
public:
    LedgeCollider() = default;
    ~LedgeCollider() override = default;

    /*~ WorldCollider Interface ~*/
    void SetBoxCollider(const Vec2& center, const Vec2& size);
    void SetLineCollider(const Vec2& start, const Vec2& end);

    /*~ LedgeCollider Interface ~*/
    void SetCliffDirection(int dir) { cliffDirection = dir; }
    int GetCliffDirection() const { return cliffDirection; }

private:
    int cliffDirection = 0;  // -1=왼쪽 절벽, 0=양방향, 1=오른쪽 절벽
};
