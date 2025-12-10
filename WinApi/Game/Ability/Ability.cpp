#include "pch.h"
#include "Ability.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Manager/CEventBus.h"

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
}

DelegateHandle Ability::WaitEvent(EGameEvent eventType, function<void(CGameObject*)> callback)
{
	if (!abilitySystem)
		return 0;

	DelegateHandle handle = abilitySystem->OnEvent.Add(
		[eventType, callback](EGameEvent triggeredEvent, CGameObject* source)
		{
			if (triggeredEvent == eventType)
				callback(source);
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

CAnimator* Ability::GetAnimator() const
{
	if (!cachedAnimator && owner)
	{
		cachedAnimator = owner->GetComponent<CAnimator>();
	}
	return cachedAnimator;
}

CRigidbody* Ability::GetRigidbody() const
{
	if (!cachedRigidbody && owner)
	{
		cachedRigidbody = owner->GetComponent<CRigidbody>();
	}
	return cachedRigidbody;
}

CBoxCollider* Ability::GetCollider() const
{
	if (!cachedCollider && owner)
	{
		cachedCollider = owner->GetComponent<CBoxCollider>();
	}
	return cachedCollider;
}

CStateSystem* Ability::GetStateSystem() const
{
	if (!cachedStateSystem && owner)
	{
		cachedStateSystem = owner->GetComponent<CStateSystem>();
	}
	return cachedStateSystem;
}

CStatComponent* Ability::GetStatComponent() const
{
	if (!cachedStatComponent && owner)
	{
		cachedStatComponent = owner->GetComponent<CStatComponent>();
	}
	return cachedStatComponent;
}

void Ability::PlaySFX(const wstring& key)
{
	EVENT->OnPlaySFX.Broadcast(owner, key);
}

void Ability::PlayBGM(const wstring& key, float volume)
{
	EVENT->OnPlayBGM.Broadcast(owner, key, volume);
}

void Ability::StopBGM()
{
	EVENT->OnStopBGM.Broadcast(owner);
}

void Ability::SpawnVFX(const wstring& key, Vec2 pos, int direction)
{
	EVENT->OnSpawnVFX.Broadcast(owner, key, pos, direction);
}

void Ability::ShakeCamera(const FShakeParams& params)
{
	EVENT->OnCameraShake.Broadcast(owner, params);
}

void Ability::SetTimeScale(float scale, float duration)
{
	EVENT->OnSetTimeScale.Broadcast(owner, scale, duration);
}
