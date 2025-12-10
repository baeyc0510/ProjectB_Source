#pragma once
#include <string>
#include "Engine/Managers/CameraManager.h"  // for FShakeParams

/*~ SFX Events ~*/
struct FSFXEventData
{
	std::wstring key;
};

struct FBGMEventData
{
	std::wstring key;
	float volume = 1.f;
};

/*~ VFX Events ~*/
struct FVFXEventData
{
	std::wstring key;
	Vec2 pos;
	int direction = 1;
};

/*~ Camera Events ~*/
struct FCameraShakeEventData
{
	FShakeParams params;
};

struct FCameraFadeEventData
{
	float duration = 0.5f;
};

/*~ Time Events ~*/
struct FTimeScaleEventData
{
	float scale = 1.f;
	float duration = 0.f;
};
