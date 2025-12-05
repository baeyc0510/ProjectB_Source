#pragma once
#include "CWorldCollider.h"

// 사다리 - Ladder 레이어
class CLadder : public CWorldCollider
{
public:
	CLadder() = default;
	~CLadder() override = default;

	/*~ CWorldCollider Interface ~*/
	void SetBoxCollider(const Vec2& center, const Vec2& size);

	/*~ CLadder Interface ~*/
	float GetLadderX() const { return ladderX; }
	float GetLadderTopY() const { return ladderTopY; }
	float GetLadderBottomY() const { return ladderBottomY; }

protected:
	/*~ CGameObject Interface ~*/
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;

private:
	float ladderX = 0.f;
	float ladderTopY = 0.f;
	float ladderBottomY = 0.f;
};
