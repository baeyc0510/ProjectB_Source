#include "pch.h"
#include "CLedge.h"

#include "CWorldCollider.h"

void CLedge::SetBoxCollider(const Vec2& center, const Vec2& size)
{
    CWorldCollider::SetBoxCollider(center,size,ELayer::Ledge);
}

void CLedge::SetLineCollider(const Vec2& start, const Vec2& end)
{
    CWorldCollider::SetLineCollider(start,end,ELayer::Ledge);
}
