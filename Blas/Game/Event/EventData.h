#pragma once
#include <string>

/*~ SFX Events ~*/
struct SFXEventData
{
	std::wstring key;
};

struct BGMEventData
{
	std::wstring key;
	float volume = 1.f;
};

/*~ VFX Events ~*/
struct VFXEventData
{
	std::wstring key;
	Vec2 pos;
	int direction = 1;
};

/*~ Camera Events ~*/
struct CameraShakeEventData
{
	FShakeParams params;
};

struct CameraFadeEventData
{
	float duration = 0.5f;
};

/*~ Time Events ~*/
struct TimeScaleEventData
{
	float scale = 1.f;
	float duration = 0.f;
};
