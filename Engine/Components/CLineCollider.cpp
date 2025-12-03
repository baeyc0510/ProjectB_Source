#include "pch.h"
#include "CLineCollider.h"

CLineCollider::CLineCollider()
	: localStart(0, 0)
	, localEnd(0, 0)
{
}

CLineCollider::~CLineCollider()
{
}

void CLineCollider::SetLine(const Vec2& start, const Vec2& end)
{
	localStart = start;
	localEnd = end;
}

Vec2 CLineCollider::GetWorldStart() const
{
	return const_cast<CLineCollider*>(this)->GetPos() + localStart;
}

Vec2 CLineCollider::GetWorldEnd() const
{
	return const_cast<CLineCollider*>(this)->GetPos() + localEnd;
}

float CLineCollider::GetYAt(float worldX) const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float dx = end.x - start.x;
	if (abs(dx) < 0.001f)
		return start.y;

	float t = (worldX - start.x) / dx;
	t = max(0.0f, min(1.0f, t));
	return start.y + t * (end.y - start.y);
}

bool CLineCollider::IsInXRange(float worldX) const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float minX = min(start.x, end.x);
	float maxX = max(start.x, end.x);
	return worldX >= minX && worldX <= maxX;
}

bool CLineCollider::IsCollisionWithBox(const Vec2& boxPos, const Vec2& boxScale) const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float boxLeft = boxPos.x - boxScale.x * 0.5f;
	float boxRight = boxPos.x + boxScale.x * 0.5f;
	float boxTop = boxPos.y - boxScale.y * 0.5f;
	float boxBottom = boxPos.y + boxScale.y * 0.5f;

	// 선분의 X 범위가 박스와 겹치는지 확인
	float lineMinX = min(start.x, end.x);
	float lineMaxX = max(start.x, end.x);

	if (lineMaxX < boxLeft || lineMinX > boxRight)
		return false;

	// 박스 범위 내의 선분 Y 값들 확인
	float checkMinX = max(lineMinX, boxLeft);
	float checkMaxX = min(lineMaxX, boxRight);

	float y1 = GetYAt(checkMinX);
	float y2 = GetYAt(checkMaxX);

	float lineMinY = min(y1, y2);
	float lineMaxY = max(y1, y2);

	// 선분의 Y 범위가 박스와 겹치는지 확인
	return !(lineMaxY < boxTop || lineMinY > boxBottom);
}

bool CLineCollider::IsPointOnLine(const Vec2& point, float toleranceY) const
{
	if (!IsInXRange(point.x))
		return false;

	float lineY = GetYAt(point.x);
	return abs(point.y - lineY) <= toleranceY;
}

bool CLineCollider::IsCollision(CCollider* other)
{
	// 다른 콜라이더가 박스 콜라이더인 경우
	return IsCollisionWithBox(other->GetPos(), other->GetScale());
}

void CLineCollider::Render()
{
	Vec2 start = CAMERA->WorldToScreenPoint(GetWorldStart());
	Vec2 end = CAMERA->WorldToScreenPoint(GetWorldEnd());

	RENDER->SetPen(PenType::Solid, RGB(0, 200, 255));
	RENDER->Line(start.x, start.y, end.x, end.y);

	// 끝점 표시
	RENDER->SetBrush(BrushType::Solid, RGB(0, 200, 255));
	RENDER->Ellipse(start.x - 3, start.y - 3, start.x + 3, start.y + 3);
	RENDER->Ellipse(end.x - 3, end.y - 3, end.x + 3, end.y + 3);

	RENDER->SetPen();
	RENDER->SetBrush();
}
