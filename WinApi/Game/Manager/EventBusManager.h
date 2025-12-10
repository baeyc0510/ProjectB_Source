#pragma once

class CGameObject;
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
	MulticastDelegate<CGameObject*, const wstring&> OnPlaySFX;
	MulticastDelegate<CGameObject*, const wstring&, float> OnPlayBGM;  // key, volume
	MulticastDelegate<CGameObject*> OnStopBGM;

	// VFX Events
	MulticastDelegate<CGameObject*, const wstring&, Vec2, int> OnSpawnVFX;  // key, pos, direction

	// Camera Events
	MulticastDelegate<CGameObject*, const FShakeParams&> OnCameraShake;
	MulticastDelegate<CGameObject*, float> OnCameraFadeIn;   // duration
	MulticastDelegate<CGameObject*, float> OnCameraFadeOut;  // duration

	// Time Events
	MulticastDelegate<CGameObject*, float, float> OnSetTimeScale;  // scale, duration

private:
	EventBusManager() = default;
	~EventBusManager() = default;
	EventBusManager(const EventBusManager&) = delete;
	EventBusManager& operator=(const EventBusManager&) = delete;
};

#define EVENT EventBusManager::GetInstance()
