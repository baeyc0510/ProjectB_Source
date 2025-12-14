#include "pch.h"
#include "WorldCollider.h"
#include "Components/LineCollider.h"
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

	collider = new BoxCollider();
	collider->SetScale(size);
	collider->SetLayer((UINT)layer);
	AddChild(collider);
}

void WorldCollider::SetLineCollider(const Vec2& start, const Vec2& end, ELayer layer)
{
	isLineCollider = true;
	SetPos(Vec2(0, 0));

	LineCollider* lineCollider = new LineCollider();
	lineCollider->SetLine(start, end);
	lineCollider->SetLayer((int)layer);
	AddChild(lineCollider);

	collider = lineCollider;
}
