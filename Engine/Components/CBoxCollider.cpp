#include "pch.h"
#include "CBoxCollider.h"

CBoxCollider::CBoxCollider()
{
	scale = Vec2(0, 0);
}

CBoxCollider::~CBoxCollider()
{
}

void CBoxCollider::RenderDebug()
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

void CBoxCollider::Render()
{
}
