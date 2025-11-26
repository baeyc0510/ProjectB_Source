#include "pch.h"
#include "Ability.h"
#include "Game/Component/CAbilitySystem.h"

Ability::Ability()
	: owner(nullptr)
	, abilitySystem(nullptr)
	, isActive(false)
	, cooldownRemaining(0.f)
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

void Ability::UpdateCooldown(float deltaTime)
{
	if (cooldownRemaining > 0.f)
		cooldownRemaining -= deltaTime;
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
	cooldownRemaining = GetCooldown();
	ClearEventHandles();
	OnEnded.Invoke();
}

void Ability::Cancel()
{
	if (!isActive)
		return;

	isActive = false;
	cooldownRemaining = GetCooldown();
	ClearEventHandles();
	OnEnded.Invoke();
}

DelegateHandle Ability::WaitEvent(EGameEvent eventType, function<void()> callback)
{
	if (!abilitySystem)
		return 0;

	DelegateHandle handle = abilitySystem->OnEvent.Add(
		[eventType, callback](EGameEvent triggeredEvent)
		{
			if (triggeredEvent == eventType)
				callback();
		}
	);

	eventHandles.push_back(handle);
	return handle;
}

void Ability::StopWaitingEvent(DelegateHandle handle)
{
	if (!abilitySystem || handle == 0)
		return;

	auto iter = find(eventHandles.begin(), eventHandles.end(), handle);
	if (iter != eventHandles.end())
	{
		abilitySystem->OnEvent.Remove(*iter);
		eventHandles.erase(iter);
	}
}

void Ability::ClearEventHandles()
{
	if (!abilitySystem)
		return;

	for (DelegateHandle handle : eventHandles)
	{
		abilitySystem->OnEvent.Remove(handle);
	}
	eventHandles.clear();
}
