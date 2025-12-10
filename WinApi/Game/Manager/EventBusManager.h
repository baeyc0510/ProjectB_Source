#pragma once

class GameObject;
struct FShakeParams;

class EventBusManager
{
public:
	static EventBusManager* GetInstance()
	{
		static EventBusManager instance;
		return &instance;
	}

	// SFX Events
	MulticastDelegate<GameObject*, const wstring&> OnPlaySFX;
	MulticastDelegate<GameObject*, const wstring&, float> OnPlayBGM;  // key, volume
	MulticastDelegate<GameObject*> OnStopBGM;

	// VFX Events
	MulticastDelegate<GameObject*, const wstring&, Vec2, int> OnSpawnVFX;  // key, pos, direction

	// Camera Events
	MulticastDelegate<GameObject*, const FShakeParams&> OnCameraShake;
	MulticastDelegate<GameObject*, float> OnCameraFadeIn;   // duration
	MulticastDelegate<GameObject*, float> OnCameraFadeOut;  // duration

	// Time Events
	MulticastDelegate<GameObject*, float, float> OnSetTimeScale;  // scale, duration

private:
	EventBusManager() = default;
	~EventBusManager() = default;
	EventBusManager(const EventBusManager&) = delete;
	EventBusManager& operator=(const EventBusManager&) = delete;
};

#define EVENT EventBusManager::GetInstance()
