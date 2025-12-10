#pragma once
#include "Game/Event/EventData.h"

class GameObject;

class EventBusManager
{
public:
	static EventBusManager* GetInstance()
	{
		static EventBusManager instance;
		return &instance;
	}

	/*~ SFX Events ~*/
	MulticastDelegate<GameObject*, const FSFXEventData&> OnPlaySFX;
	MulticastDelegate<GameObject*, const FBGMEventData&> OnPlayBGM;
	MulticastDelegate<GameObject*> OnStopBGM;

	/*~ VFX Events ~*/
	MulticastDelegate<GameObject*, const FVFXEventData&> OnSpawnVFX;

	/*~ Camera Events ~*/
	MulticastDelegate<GameObject*, const FCameraShakeEventData&> OnCameraShake;
	MulticastDelegate<GameObject*, const FCameraFadeEventData&> OnCameraFadeIn;
	MulticastDelegate<GameObject*, const FCameraFadeEventData&> OnCameraFadeOut;

	/*~ Time Events ~*/
	MulticastDelegate<GameObject*, const FTimeScaleEventData&> OnSetTimeScale;

private:
	EventBusManager() = default;
	~EventBusManager() = default;
	EventBusManager(const EventBusManager&) = delete;
	EventBusManager& operator=(const EventBusManager&) = delete;
};

#define EVENT EventBusManager::GetInstance()
