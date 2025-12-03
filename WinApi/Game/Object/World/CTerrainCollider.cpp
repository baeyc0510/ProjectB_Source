#include "pch.h"
#include "CTerrainCollider.h"
#include "Components/CLineCollider.h"
#include "Game/Enum.h"

CTerrainCollider::CTerrainCollider()
{
}

CTerrainCollider::~CTerrainCollider()
{
}

void CTerrainCollider::SetBoxCollider(const Vec2& center, const Vec2& size)
{
	isLineCollider = false;
	SetPos(center);

	collider = new CCollider();
	collider->SetScale(size);
	collider->SetLayer(ELayer::Ground);
	AddChild(collider);
}

void CTerrainCollider::SetLineCollider(const Vec2& start, const Vec2& end)
{
	isLineCollider = true;
	SetPos(Vec2(0, 0));  // 라인 콜라이더는 절대 좌표 사용

	CLineCollider* lineCollider = new CLineCollider();
	lineCollider->SetLine(start, end);
	lineCollider->SetLayer(ELayer::Ground);
	AddChild(lineCollider);

	collider = lineCollider;
}

void CTerrainCollider::Init()
{
}

void CTerrainCollider::OnEnable()
{
}

void CTerrainCollider::Update()
{
}

void CTerrainCollider::OnDisable()
{
}

void CTerrainCollider::Release()
{
}

void CTerrainCollider::Render()
{
}
