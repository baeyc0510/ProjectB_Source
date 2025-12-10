#include "pch.h"
#include "VFXManager.h"
#include "Game/Object/VFXObject.h"
#include "Game/VFXKeys.h"

void VFXManager::PreLoad()
{
    LOADANIMATION(VFXKey::AttackHit1,TEXT("Animations/Effects/attack_spark1.json"));
    LOADANIMATION(VFXKey::AttackHit2,TEXT("Animations/Effects/attack_spark2.json"));
    LOADANIMATION(VFXKey::AttackHit3,TEXT("Animations/Effects/attack_spark3.json"));
    LOADANIMATION(VFXKey::PlayerHit,TEXT("Animations/Effects/playerhitspark.json"));
    LOADANIMATION(VFXKey::Blood1,TEXT("Animations/Effects/blood1.json"));
    LOADANIMATION(VFXKey::Blood2,TEXT("Animations/Effects/blood2.json"));
    
    // 타이틀 배경
    LOADANIMATION(VFXKey::Title_Bg, TEXT("Animations/MainMenu/crisanta-bg.json"));
    LOADANIMATION(VFXKey::Title_Main, TEXT("Animations/MainMenu/crisanta-main-menu-anim.json"));
    LOADANIMATION(VFXKey::Title_Petals, TEXT("Animations/MainMenu/crisanta-petals.json"));
    LOADANIMATION(VFXKey::Title_Fg, TEXT("Animations/MainMenu/crisanta-fg-petals.json"));
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
