#include "pch.h"
#include "SFXManager.h"
#include "Game/Data/SFXData.h"

void SFXManager::PreLoad()
{
    for (const auto& sound : SFXData::GetAllSounds())
    {
        LOADSOUND(sound.key, sound.path);
    }
}

void SFXManager::PlayOnce(const wstring& key, float volume)
{
    SoundResource* sound = GetSound(key);
    if (sound)
    {
        SOUND->PlayOnce(sound, volume);
    }
}

void SFXManager::PlayLoop(const wstring& key, float volume)
{
    SoundResource* sound = GetSound(key);
    if (sound)
    {
        SOUND->PlayLoop(key, sound, volume);
    }
}

void SFXManager::Stop(const wstring& key)
{
    SOUND->Stop(key);
}

void SFXManager::StopAll()
{
    SOUND->StopAll();
}

void SFXManager::PlayBGM(const wstring& key, float volume)
{
    if (currentBGM == key)
        return;
    
    Stop(currentBGM);
    currentBGM = key;
    PlayLoop(key, volume);
}

void SFXManager::StopBGM()
{
    if (currentBGM.empty())
        return;
    
    Stop(currentBGM);
}

void SFXManager::PlayAmbient(const wstring& key, float volume)
{
    if (currentAmbient == key)
        return;
    
    Stop(currentAmbient);
    currentAmbient = key;
    PlayLoop(key, volume);
}

void SFXManager::StopAmbient()
{
    if (currentAmbient.empty())
        return;
    
    Stop(currentAmbient);
}

SoundResource* SFXManager::GetSound(const wstring& key)
{
    return SINGLE(ResourceManager)->SoundFind(key);
}
