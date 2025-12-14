#pragma once
#include "Game/Data/SFXKeys.h"

class SoundResource;

class SFXManager : public SingleTon<SFXManager>
{
    friend SingleTon<SFXManager>;

public:
    void PreLoad();

    void PlayOnce(const wstring& key, float volume = 1.f);
    void PlayLoop(const wstring& key, float volume = 1.f);
    
    void PlayBGM(const wstring& key, float volume = 1.f);
    void StopBGM();
    
    void PlayAmbient(const wstring& key, float volume = 1.f);
    void StopAmbient();
    
    void Stop(const wstring& key);
    void StopAll();

private:
    SoundResource* GetSound(const wstring& key);
    
private:
    wstring currentBGM;
    wstring currentAmbient;
};

#define SFX SFXManager::GetInstance()
