#pragma once
#include "CWorldCollider.h"

// 지면 콜라이더 - Ground 레이어
class CGroundCollider : public CWorldCollider
{
public:
	CGroundCollider() = default;
	~CGroundCollider() override = default;

	void SetBoxCollider(const Vec2& center, const Vec2& size);
	void SetLineCollider(const Vec2& start, const Vec2& end);
};
