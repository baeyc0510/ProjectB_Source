#include "pch.h"
#include "CVFXManager.h"

#include "Game/Object/CVFX.h"

void CVFXManager::PreLoad()
{
    LOADANIMATION(TEXT("VFX_Attack1"),TEXT("Animations/Effects/attack_spark1.json"));
    LOADANIMATION(TEXT("VFX_Attack2"),TEXT("Animations/Effects/attack_spark2.json"));
    LOADANIMATION(TEXT("VFX_Attack3"),TEXT("Animations/Effects/attack_spark3.json"));
}

CVFX* CVFXManager::CreateVFX(wstring key)
{
    currentScene = SINGLE(CSceneManager)->GetCurScene();
    if (currentScene == nullptr)
    {
        return nullptr;
    }
    
    // Spawn VFX
    CAnimation* anim = SINGLE(CResourceManager)->AnimationFind(key);
    assert(anim);
    
    CVFX* vfx = new CVFX();
    currentScene->AddGameObject(vfx);
    vfx->SetAnimation(anim);
    
    return vfx;
}
