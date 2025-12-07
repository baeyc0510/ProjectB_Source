#pragma once
#include "CWorldCollider.h"

// 지면 콜라이더 - Ground 레이어
class CGround : public CWorldCollider
{
public:
	CGround() = default;
	~CGround() override = default;

	/*~ CWorldCollider Interface ~*/
	void SetBoxCollider(const Vec2& center, const Vec2& size);
	void SetLineCollider(const Vec2& start, const Vec2& end);
};