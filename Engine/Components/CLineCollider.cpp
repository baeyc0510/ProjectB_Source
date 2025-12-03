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

Vec2 CLineCollider::GetScale() const
{
	// 선분의 바운딩 박스 크기 반환
	float width = abs(localEnd.x - localStart.x);
	float height = abs(localEnd.y - localStart.y);
	return Vec2(width, height);
}

void CLineCollider::SetScale(const Vec2& scale)
{
	// 라인 콜라이더는 SetLine으로 설정하므로 무시
}

Vec2 CLineCollider::GetWorldStart() const
{
	return GetPos() + localStart;
}

Vec2 CLineCollider::GetWorldEnd() const
{
	return GetPos() + localEnd;
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

float CLineCollider::GetXAt(float worldY) const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float dy = end.y - start.y;
	if (abs(dy) < 0.001f)
		return start.x;

	float t = (worldY - start.y) / dy;
	t = max(0.0f, min(1.0f, t));
	return start.x + t * (end.x - start.x);
}

bool CLineCollider::IsInXRange(float worldX) const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float minX = min(start.x, end.x);
	float maxX = max(start.x, end.x);
	return worldX >= minX && worldX <= maxX;
}

bool CLineCollider::IsPointOnLine(const Vec2& point, float toleranceY) const
{
	if (!IsInXRange(point.x))
		return false;

	float lineY = GetYAt(point.x);
	return abs(point.y - lineY) <= toleranceY;
}

float CLineCollider::GetSlopeAngle() const
{
	Vec2 start = GetWorldStart();
	Vec2 end = GetWorldEnd();

	float dx = end.x - start.x;
	float dy = end.y - start.y;

	return atan2f(-dy, abs(dx));
}

float CLineCollider::GetSlopeAngleDegrees() const
{
	constexpr float RAD_TO_DEG = 180.0f / 3.14159265f;
	return GetSlopeAngle() * RAD_TO_DEG;
}

void CLineCollider::Render()
{
	Vec2 start = CAMERA->WorldToScreenPoint(GetWorldStart());
	Vec2 end = CAMERA->WorldToScreenPoint(GetWorldEnd());

	COLORREF color = IsColliding() ? RGB(255, 0, 0) : layerColors[GetLayer() % 16];

	RENDER->SetPen(PenType::Solid, color);
	RENDER->Line(start.x, start.y, end.x, end.y);

	// 끝점 표시
	RENDER->SetBrush(BrushType::Solid, color);
	RENDER->Ellipse(start.x - 3, start.y - 3, start.x + 3, start.y + 3);
	RENDER->Ellipse(end.x - 3, end.y - 3, end.x + 3, end.y + 3);

	RENDER->SetPen();
	RENDER->SetBrush();
}
