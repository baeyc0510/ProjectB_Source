#pragma once
#include "CWorldCollider.h"

// 플랫폼 - Platform 레이어 (원웨이 플랫폼)
class CPlatform : public CWorldCollider
{
public:
	CPlatform() = default;
	~CPlatform() override = default;

	void SetBoxCollider(const Vec2& center, const Vec2& size);
	void SetLineCollider(const Vec2& start, const Vec2& end);
};
