#include "pch.h"
#include "LedgeCollider.h"

#include "WorldCollider.h"

void LedgeCollider::SetBoxCollider(const Vec2& center, const Vec2& size)
{
    WorldCollider::SetBoxCollider(center,size,ELayer::Ledge);
}

void LedgeCollider::SetLineCollider(const Vec2& start, const Vec2& end)
{
    WorldCollider::SetLineCollider(start,end,ELayer::Ledge);
}
