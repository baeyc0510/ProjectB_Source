#include "pch.h"
#include "AIControllerBase.h"
#include "AbilitySystem.h"
#include "StateSystem.h"
#include "Game/Object/Character/Player.h"

void AIControllerBase::CacheComponents()
{
	stateSystem = owner->GetComponent<StateSystem>();
	abilitySystem = owner->GetComponent<AbilitySystem>();

	assert(stateSystem && "AIControllerBase requires StateSystem");
	assert(abilitySystem && "AIControllerBase requires AbilitySystem");
}

void AIControllerBase::FindPlayer()
{
	if (!owner || !owner->GetScene())
		return;

	target = owner->GetScene()->FindObjectByType<Player>();
}

float AIControllerBase::GetDistanceToTarget() const
{
	if (!target || !owner)
		return FLT_MAX;

	Vec2 ownerPos = owner->GetPos();
	Vec2 targetPos = target->GetPos();
	Vec2 diff = targetPos - ownerPos;
	return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

float AIControllerBase::GetDistanceToTargetY() const
{
	if (!target || !owner)
		return FLT_MAX;

	return abs(target->GetPos().y - owner->GetPos().y);
}

int AIControllerBase::GetDirectionToTarget() const
{
	if (!target || !owner)
		return 1;

	float diff = target->GetPos().x - owner->GetPos().x;
	return diff >= 0 ? 1 : -1;
}
