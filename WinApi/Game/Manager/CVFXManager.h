#pragma once

class CVFX;

class CVFXManager : public SingleTon<CVFXManager>
{
    friend SingleTon<CVFXManager>;

public:
    void PreLoad();
    CVFX* CreateVFX(wstring key);
    
private:
    CScene* currentScene;
};

#define VFX	CVFXManager::GetInstance()
