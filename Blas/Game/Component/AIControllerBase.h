#pragma once
#include "Core/Component.h"
#include "Core/GameObject.h"

class Player;
class AbilitySystem;
class StateSystem;

class AIControllerBase : public Component<GameObject>
{
public:
	AIControllerBase() = default;
	virtual ~AIControllerBase() = default;

	/*~ Target Interface ~*/
	GameObject* GetTarget() const { return target; }
	bool HasTarget() const { return target != nullptr; }

	/*~ Distance/Direction Utilities ~*/
	virtual float GetDistanceToTarget() const;
	float GetDistanceToTargetY() const;
	int GetDirectionToTarget() const;	// -1 or 1

protected:
	void FindPlayer();

protected:
	GameObject* target = nullptr;
};
