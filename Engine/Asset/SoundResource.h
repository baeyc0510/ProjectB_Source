#pragma once
#include "ResourceBase.h"
class SoundResource : public ResourceBase
{
public:
	SoundResource();
	virtual ~SoundResource();

public:
	void Load(const wstring& path);

	FMOD::Sound* GetSound() { return sound; }

private:
	FMOD::Sound* sound;
};

