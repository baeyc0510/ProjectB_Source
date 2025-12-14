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
	MulticastDelegate<GameObject*, const SFXEventData&> OnPlaySFX;
	MulticastDelegate<GameObject*, const BGMEventData&> OnPlayBGM;
	MulticastDelegate<GameObject*> OnStopBGM;

	/*~ VFX Events ~*/
	MulticastDelegate<GameObject*, const VFXEventData&> OnSpawnVFX;

	/*~ Camera Events ~*/
	MulticastDelegate<GameObject*, const CameraShakeEventData&> OnCameraShake;
	MulticastDelegate<GameObject*, const CameraFadeEventData&> OnCameraFadeIn;
	MulticastDelegate<GameObject*, const CameraFadeEventData&> OnCameraFadeOut;

	/*~ Time Events ~*/
	MulticastDelegate<GameObject*, const TimeScaleEventData&> OnSetTimeScale;

private:
	EventBusManager() = default;
	~EventBusManager() = default;
	EventBusManager(const EventBusManager&) = delete;
	EventBusManager& operator=(const EventBusManager&) = delete;
};

#define EVENT EventBusManager::GetInstance()
