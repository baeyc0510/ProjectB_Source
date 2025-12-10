#include "pch.h"
#include "AIUtils.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Game/Object/Character/Player.h"

namespace AIUtils
{
	GameObject* FindPlayer(Scene* scene)
	{
		if (!scene)
			return nullptr;

		return scene->FindObjectByType<Player>();
	}

	float GetDistanceX(GameObject* from, GameObject* to)
	{
		if (!from || !to)
			return FLT_MAX;

		return abs(to->GetPos().x - from->GetPos().x);
	}

	float GetDistanceY(GameObject* from, GameObject* to)
	{
		if (!from || !to)
			return FLT_MAX;

		return abs(to->GetPos().y - from->GetPos().y);
	}

	float GetDistance2D(GameObject* from, GameObject* to)
	{
		if (!from || !to)
			return FLT_MAX;

		Vec2 diff = to->GetPos() - from->GetPos();
		return sqrtf(diff.x * diff.x + diff.y * diff.y);
	}

	int GetDirectionX(GameObject* from, GameObject* to)
	{
		if (!from || !to)
			return 1;

		float diff = to->GetPos().x - from->GetPos().x;
		return diff >= 0 ? 1 : -1;
	}

	bool IsInRange(GameObject* from, GameObject* to, float range)
	{
		return GetDistance2D(from, to) <= range;
	}

	bool IsInRangeXY(GameObject* from, GameObject* to, float rangeX, float rangeY)
	{
		return GetDistanceX(from, to) <= rangeX && GetDistanceY(from, to) <= rangeY;
	}
}
