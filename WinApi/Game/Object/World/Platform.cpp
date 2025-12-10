#include "pch.h"
#include "Platform.h"
#include "Game/Enum.h"

void Platform::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	WorldCollider::SetBoxCollider(center, size, ELayer::Platform);
}

void Platform::SetLineCollider(const Vec2& start, const Vec2& end)
{
	WorldCollider::SetLineCollider(start, end, ELayer::Platform);
}
