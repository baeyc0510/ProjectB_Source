#include "pch.h"
#include "CGround.h"
#include "Game/Enum.h"

void CGround::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	CWorldCollider::SetBoxCollider(center, size, ELayer::Ground);
}

void CGround::SetLineCollider(const Vec2& start, const Vec2& end)
{
	CWorldCollider::SetLineCollider(start, end, ELayer::Ground);
}
