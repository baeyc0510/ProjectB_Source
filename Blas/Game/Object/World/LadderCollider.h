#pragma once
#include "WorldCollider.h"

// 사다리 - Ladder 레이어
class LadderCollider : public WorldCollider
{
public:
	LadderCollider() = default;
	~LadderCollider() override = default;

	/*~ WorldCollider Interface ~*/
	void SetBoxCollider(const Vec2& center, const Vec2& size);

	/*~ Ladder Interface ~*/
	float GetLadderX() const { return ladderX; }
	float GetLadderTopY() const { return ladderTopY; }
	float GetLadderBottomY() const { return ladderBottomY; }

protected:
	/*~ GameObject Interface ~*/
	void OnCollisionEnter(Collider* other) override;
	void OnCollisionExit(Collider* other) override;

private:
	float ladderX = 0.f;
	float ladderTopY = 0.f;
	float ladderBottomY = 0.f;
};
