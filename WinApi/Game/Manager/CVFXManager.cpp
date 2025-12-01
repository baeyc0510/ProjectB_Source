#include "pch.h"
#include "CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Game/VFXKeys.h"

void CVFXManager::PreLoad()
{
    LOADANIMATION(VFXKey::AttackHit1,TEXT("Animations/Effects/attack_spark1.json"));
    LOADANIMATION(VFXKey::AttackHit2,TEXT("Animations/Effects/attack_spark2.json"));
    LOADANIMATION(VFXKey::AttackHit3,TEXT("Animations/Effects/attack_spark3.json"));
    LOADANIMATION(VFXKey::PlayerHit,TEXT("Animations/Effects/playerhitspark.json"));
    LOADANIMATION(VFXKey::Blood1,TEXT("Animations/Effects/blood1.json"));
    LOADANIMATION(VFXKey::Blood2,TEXT("Animations/Effects/blood2.json"));
}

CVFX* CVFXManager::CreateVFX(const wstring& key)
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

CVFX* CVFXManager::CreateVFX(const wstring& key, Vec2 pos, int direction)
{
    CVFX* vfx = CreateVFX(key);
    if (vfx)
    {
        vfx->SetPos(pos);
        vfx->SetForward(direction);
    }
    
    return vfx;
}
