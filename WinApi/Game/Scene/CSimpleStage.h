#pragma once
#include "CMapScene.h"

class CSimpleStage : public CMapScene
{
public:
    CSimpleStage();
    CSimpleStage(const wstring& inMapPath);
    ~CSimpleStage();
    
    void SetMapFilePath(const wstring& path);
    
protected:
    void Init() override;
    void OnLoadMap() override;
    
    void Enter() override;
};
