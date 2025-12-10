#include "pch.h"
#include "CLedgeHelper.h"
#include "Game/Object/World/CLedge.h"

void CLedgeHelper::CheckLedge(CCollider* other, Vec2 playerPos, Vec2 playerHalfScale)
{
	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;
	float otherTop = otherPos.y - otherHalf.y;

	// 이미 같은 ledge로 마크되어 있는 경우 early return
	if (state.bOverlapWithLedge && state.ledgeId == other->GetID())
		return;

	// 플레이어가 ledge 위에 서 있으면 무시
	float playerBottom = playerPos.y + playerHalfScale.y;
	if (playerBottom < otherTop)
		return;

	// 이미 겹쳐있는 다른 Ledge가 있고 해당 ledge보다 낮으면 갱신 x
	if (state.bOverlapWithLedge && otherTop > state.ledgeTop)
		return;

	// 방향 계산: ledge가 플레이어 기준 왼쪽(-1) 또는 오른쪽(1)
	int direction = (otherPos.x > playerPos.x) ? 1 : -1;

	// CLedge의 절벽 방향 확인
	CLedge* ledge = dynamic_cast<CLedge*>(other->GetOwner());
	if (ledge)
	{
		int cliffDir = ledge->GetCliffDirection();
		// 절벽 방향이 지정되어 있고 플레이어 방향과 맞지 않으면 무시
		if (cliffDir != 0 && cliffDir != -direction)
			return;
	}

	// Ledge 상태 갱신
	state.bOverlapWithLedge = true;
	state.ledgeId = other->GetID();
	state.ledgeTop = otherTop;
	state.ledgeX = otherPos.x;
	state.ledgeDirection = direction;
}

bool CLedgeHelper::ShouldClearOnExit(CCollider* other) const
{
	return state.ledgeId == other->GetID();
}
