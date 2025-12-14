#include "pch.h"
#include "VFXManager.h"
#include "Game/Object/VFXObject.h"
#include "Game/Data/VFXData.h"

void VFXManager::PreLoad()
{
    for (const auto& vfx : VFXData::GetAllVFX())
    {
        LOADANIMATION(vfx.key, vfx.path);
    }
}

VFXObject* VFXManager::CreateVFX(const wstring& key)
{
    Scene* currentScene = SINGLE(SceneManager)->GetCurScene();
    if (currentScene == nullptr)
    {
        return nullptr;
    }
    
    // Spawn VFX
    AnimationResource* anim = SINGLE(ResourceManager)->AnimationFind(key);
    assert(anim);
    
    VFXObject* vfx = new VFXObject();
    currentScene->AddGameObject(vfx);
    vfx->SetAnimation(anim);
    
    return vfx;
}

VFXObject* VFXManager::CreateVFX(const wstring& key, Vec2 pos, int direction)
{
    VFXObject* vfx = CreateVFX(key);
    if (vfx)
    {
        vfx->SetPos(pos);
        vfx->SetForward(direction);
    }
    
    return vfx;
}
