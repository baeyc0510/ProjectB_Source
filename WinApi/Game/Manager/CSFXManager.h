#pragma once
#include "Game/SFXKeys.h"

class CSound;

class CSFXManager : public SingleTon<CSFXManager>
{
    friend SingleTon<CSFXManager>;

public:
    void PreLoad();

    void PlayOnce(const wstring& key, float volume = 1.f);
    void PlayLoop(const wstring& key, float volume = 1.f);
    void PlayBGM(const wstring& key, float volume = 1.f);
    void Stop(const wstring& key);
    void StopAll();

private:
    CSound* GetSound(const wstring& key);
    
private:
    wstring currentBGM;
};

#define SFX CSFXManager::GetInstance()
