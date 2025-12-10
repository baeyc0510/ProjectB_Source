#include "pch.h"
#include "BoxCollider.h"

BoxCollider::BoxCollider()
{
	scale = Vec2(0, 0);
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::RenderDebug()
{
	Vec2 renderPos = CAMERA->WorldToScreenPoint(GetPos());

	COLORREF color = IsColliding() ? RGB(255, 0, 0) : layerColors[GetLayer() % 16];

	RENDER->SetPen(PenType::Solid, color);
	RENDER->SetBrush(BrushType::Null);

	RENDER->Rect(
		renderPos.x - scale.x * 0.5f,
		renderPos.y - scale.y * 0.5f,
		renderPos.x + scale.x * 0.5f,
		renderPos.y + scale.y * 0.5f);

	RENDER->SetPen();
	RENDER->SetBrush();
}

void BoxCollider::Render()
{
}
