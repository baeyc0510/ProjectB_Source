#include "pch.h"
#include "CPlatform.h"
#include "Game/Enum.h"

void CPlatform::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	CWorldCollider::SetBoxCollider(center, size, ELayer::Platform);
}

void CPlatform::SetLineCollider(const Vec2& start, const Vec2& end)
{
	CWorldCollider::SetLineCollider(start, end, ELayer::Platform);
}
