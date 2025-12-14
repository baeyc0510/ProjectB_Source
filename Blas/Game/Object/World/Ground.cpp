#include "pch.h"
#include "Ground.h"
#include "Game/Enum.h"

void Ground::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	WorldCollider::SetBoxCollider(center, size, ELayer::Ground);
}

void Ground::SetLineCollider(const Vec2& start, const Vec2& end)
{
	WorldCollider::SetLineCollider(start, end, ELayer::Ground);
}
