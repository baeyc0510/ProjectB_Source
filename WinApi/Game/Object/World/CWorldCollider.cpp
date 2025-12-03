#include "pch.h"
#include "CWorldCollider.h"
#include "Components/CLineCollider.h"
#include "Game/Enum.h"

CWorldCollider::CWorldCollider()
{
}

CWorldCollider::~CWorldCollider()
{
}

void CWorldCollider::SetBoxCollider(const Vec2& center, const Vec2& size, ELayer layer)
{
	isLineCollider = false;
	SetPos(center);

	collider = new CBoxCollider();
	collider->SetScale(size);
	collider->SetLayer(layer);
	AddChild(collider);
}

void CWorldCollider::SetLineCollider(const Vec2& start, const Vec2& end, ELayer layer)
{
	isLineCollider = true;
	SetPos(Vec2(0, 0));

	CLineCollider* lineCollider = new CLineCollider();
	lineCollider->SetLine(start, end);
	lineCollider->SetLayer(layer);
	AddChild(lineCollider);

	collider = lineCollider;
}
