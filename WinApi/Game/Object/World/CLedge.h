#pragma once
#include "CWorldCollider.h"

class CLedge : public CWorldCollider
{
public:
    CLedge() = default;
    ~CLedge() override = default;

    /*~ CWorldCollider Interface ~*/
    void SetBoxCollider(const Vec2& center, const Vec2& size);
    void SetLineCollider(const Vec2& start, const Vec2& end);
};
