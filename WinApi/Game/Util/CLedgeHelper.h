#pragma once

class CCollider;

// Ledge 상태 정보
struct FLedgeState
{
	bool bOverlapWithLedge = false;
	UINT ledgeId = 0;
	float ledgeX = -FLT_MAX;
	float ledgeTop = -FLT_MAX;
	int ledgeDirection = 0;

	void Reset()
	{
		bOverlapWithLedge = false;
		ledgeId = 0;
		ledgeX = -FLT_MAX;
		ledgeTop = -FLT_MAX;
		ledgeDirection = 0;
	}
};

// Ledge 감지 및 상태 관리 헬퍼
class CLedgeHelper
{
public:
	CLedgeHelper() = default;

	// 상태 접근
	bool IsOverlappingLedge() const { return state.bOverlapWithLedge; }
	float GetLedgeX() const { return state.ledgeX; }
	float GetLedgeTop() const { return state.ledgeTop; }
	int GetLedgeDirection() const { return state.ledgeDirection; }
	UINT GetLedgeId() const { return state.ledgeId; }

	// Ledge 충돌 체크
	void CheckLedge(CCollider* other, Vec2 playerPos, Vec2 playerHalfScale);

	// Ledge 상태 초기화
	void ClearLedge() { state.Reset(); }

	// Ledge 이탈 체크
	bool ShouldClearOnExit(CCollider* other) const;

private:
	FLedgeState state;
};
