#include "pch.h"
#include "CGroundCollider.h"
#include "Game/Enum.h"

void CGroundCollider::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	CWorldCollider::SetBoxCollider(center, size, ELayer::Ground);
}

void CGroundCollider::SetLineCollider(const Vec2& start, const Vec2& end)
{
	CWorldCollider::SetLineCollider(start, end, ELayer::Ground);
}
