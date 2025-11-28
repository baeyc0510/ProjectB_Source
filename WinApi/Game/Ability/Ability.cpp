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
	OnActivate();
}

void Ability::OnActivate()
{
	
}

void Ability::EndAbility()
{
	if (!isActive)
		return;

	isActive = false;
	cooldownRemaining = GetCooldown();
	OnEnd();
	OnEnded.Invoke();
}

void Ability::OnEnd()
{
}

void Ability::CancelAbility()
{
	if (!isActive)
		return;

	cooldownRemaining = GetCooldown();
	EndAbility();
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

void Ability::EndWaitEvent(DelegateHandle& handle)
{
	if (!abilitySystem || !handle.IsValid())
	{
		handle = DelegateHandle(); // 핸들 무효화
		return;
	}

	auto iter = find(eventHandles.begin(), eventHandles.end(), handle);
	if (iter != eventHandles.end())
	{
		abilitySystem->OnEvent.Remove(*iter);
		eventHandles.erase(iter);
	}
	
	handle = DelegateHandle(); // 핸들 무효화
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
