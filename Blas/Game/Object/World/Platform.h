#pragma once
#include "WorldCollider.h"

// 플랫폼 - Platform 레이어 (원웨이 플랫폼)
class Platform : public WorldCollider
{
public:
	Platform() = default;
	~Platform() override = default;
	
	/*~ Platform Interface ~*/
	void SetBoxCollider(const Vec2& center, const Vec2& size);
	void SetLineCollider(const Vec2& start, const Vec2& end);
};
