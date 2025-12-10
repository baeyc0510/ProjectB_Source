#include "pch.h"
#include "WorldCollider.h"
#include "Components/CLineCollider.h"
#include "Game/Enum.h"

WorldCollider::WorldCollider()
{
}

WorldCollider::~WorldCollider()
{
}

void WorldCollider::SetBoxCollider(const Vec2& center, const Vec2& size, ELayer layer)
{
	isLineCollider = false;
	SetPos(center);

	collider = new CBoxCollider();
	collider->SetScale(size);
	collider->SetLayer((UINT)layer);
	AddChild(collider);
}

void WorldCollider::SetLineCollider(const Vec2& start, const Vec2& end, ELayer layer)
{
	isLineCollider = true;
	SetPos(Vec2(0, 0));

	CLineCollider* lineCollider = new CLineCollider();
	lineCollider->SetLine(start, end);
	lineCollider->SetLayer((int)layer);
	AddChild(lineCollider);

	collider = lineCollider;
}
