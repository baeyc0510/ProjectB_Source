#include "pch.h"
#include "Ability.h"
#include "Game/Component/CAbilitySystem.h"

Ability::Ability()
	: owner(nullptr)
	, abilitySystem(nullptr)
	, isActive(false)
{
}

Ability::~Ability()
{
	ClearEventHandles();
}

void Ability::Init(CGameObject* inOwner, CAbilitySystem* inAbilitySystem)
{
	owner = inOwner;
	abilitySystem = inAbilitySystem;
}

void Ability::Activate()
{
	isActive = true;
}

void Ability::End()
{
	if (!isActive)
		return;

	isActive = false;
	ClearEventHandles();
	OnEnded.Invoke();
}

void Ability::Cancel()
{
	if (!isActive)
		return;

	isActive = false;
	ClearEventHandles();
	OnEnded.Invoke();
}

void Ability::WaitEvent(EGameEvent eventType, function<void()> callback)
{
	if (!abilitySystem)
		return;

	DelegateHandle handle = abilitySystem->OnEvent.Add(
		[eventType, callback](EGameEvent triggeredEvent)
		{
			if (triggeredEvent == eventType)
				callback();
		}
	);

	eventHandles.push_back({ eventType, handle });
}

void Ability::ClearEventHandles()
{
	if (!abilitySystem)
		return;

	for (auto& pair : eventHandles)
	{
		abilitySystem->OnEvent.Remove(pair.second);
	}
	eventHandles.clear();
}
