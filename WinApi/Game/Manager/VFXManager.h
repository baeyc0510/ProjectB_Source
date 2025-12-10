#pragma once

class VFXObject;

class VFXManager : public SingleTon<VFXManager>
{
    friend SingleTon<VFXManager>;

public:
    void PreLoad();
    VFXObject* CreateVFX(const wstring& key);
    VFXObject* CreateVFX(const wstring& key, Vec2 pos, int direction);
};

#define VFX	VFXManager::GetInstance()
