#pragma once

class GameObject;
class Scene;

namespace AIUtils
{
	/*~ Target Finding ~*/
	GameObject* FindPlayer(Scene* scene);

	/*~ Distance Calculations ~*/
	float GetDistanceX(GameObject* from, GameObject* to);
	float GetDistanceY(GameObject* from, GameObject* to);
	float GetDistance2D(GameObject* from, GameObject* to);

	/*~ Direction Calculations ~*/
	int GetDirectionX(GameObject* from, GameObject* to);  // -1 or 1

	/*~ Range Checks ~*/
	bool IsInRange(GameObject* from, GameObject* to, float range);
	bool IsInRangeXY(GameObject* from, GameObject* to, float rangeX, float rangeY);
}
