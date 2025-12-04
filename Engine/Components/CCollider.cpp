#include "pch.h"
#include "CCollider.h"

UINT CCollider::colliderCount = 0;

void CCollider::RenderDebug()
{
}

const COLORREF CCollider::layerColors[16] = {
	RGB(255, 255, 255),  // 0 - 흰색
	RGB(0, 0, 255),      // 1 - 파랑
	RGB(255, 255, 0),    // 2 - 노랑
	RGB(0, 255, 0),      // 3 - 초록
	RGB(255, 0, 255),    // 4 - 마젠타
	RGB(0, 255, 255),    // 5 - 청록
	RGB(255, 128, 0),    // 6 - 주황
	RGB(0, 128, 0),      // 7 - 진초록
	RGB(0, 128, 128),    // 8 - 청록(어두운)
	RGB(128, 0, 128),    // 9 - 보라
	RGB(128, 0, 0),      // 10 - 진빨강
	RGB(128, 128, 255),  // 11 - 연보라
	RGB(128, 128, 0),    // 12 - 올리브
	RGB(255, 192, 203),  // 13 - 분홍
	RGB(139, 69, 19),    // 14 - 갈색
	RGB(192, 192, 192),  // 15 - 회색
};

CCollider::CCollider()
{
	id		= colliderCount++;
	count	= 0;
	layer	= 0;
	pos		= Vec2(0, 0);
	offset	= Vec2(0, 0);
	zOrder	= -10;
}

CCollider::~CCollider()
{
}

void CCollider::ComponentInit()
{
}

void CCollider::ComponentOnEnable()
{
	count = 0;  // 충돌 상태 초기화
	SINGLE(CCollisionManager)->AddCollider(this);
	Component::ComponentOnEnable();
}

void CCollider::ComponentUpdate()
{
	pos = GetOwner()->GetWorldPos() + offset;
}

void CCollider::ComponentRender()
{
	GetScene()->AddRenderer(this);
}

void CCollider::ComponentOnDisable()
{
	Component::ComponentOnDisable();
	SINGLE(CCollisionManager)->RemoveCollider(this);
	count = 0;  // 충돌 상태 초기화
}

void CCollider::ComponentRelease()
{
}

void CCollider::OnCollisionEnter(CCollider* other)
{
	count++;
	GetOwner()->OnCollisionEnter(other);
}

void CCollider::OnCollisionStay(CCollider* other)
{
	GetOwner()->OnCollisionStay(other);
}

void CCollider::OnCollisionExit(CCollider* other)
{
	count--;
	GetOwner()->OnCollisionExit(other);
}
