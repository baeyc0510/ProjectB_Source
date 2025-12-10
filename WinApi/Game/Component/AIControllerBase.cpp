#include "pch.h"
#include "AIControllerBase.h"
#include "AbilitySystem.h"
#include "StateSystem.h"
#include "Game/Util/AIUtils.h"

void AIControllerBase::CacheComponents()
{
	stateSystem = owner->GetComponent<StateSystem>();
	abilitySystem = owner->GetComponent<AbilitySystem>();

	assert(stateSystem && "AIControllerBase requires StateSystem");
	assert(abilitySystem && "AIControllerBase requires AbilitySystem");
}

void AIControllerBase::FindPlayer()
{
	if (!owner)
		return;

	target = AIUtils::FindPlayer(owner->GetScene());
}

float AIControllerBase::GetDistanceToTarget() const
{
	return AIUtils::GetDistance2D(owner, target);
}

float AIControllerBase::GetDistanceToTargetY() const
{
	return AIUtils::GetDistanceY(owner, target);
}

int AIControllerBase::GetDirectionToTarget() const
{
	return AIUtils::GetDirectionX(owner, target);
}
