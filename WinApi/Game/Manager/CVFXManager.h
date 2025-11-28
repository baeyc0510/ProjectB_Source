#pragma once

class CVFX;

class CVFXManager : public SingleTon<CVFXManager>
{
    friend SingleTon<CVFXManager>;

public:
    void PreLoad();
    CVFX* CreateVFX(const wstring& key);
    CVFX* CreateVFX(const wstring& key, Vec2 pos, int direction);
    
private:
    CScene* currentScene;
};

#define VFX	CVFXManager::GetInstance()
