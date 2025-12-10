#pragma once
#include "MapScene.h"

class SimpleStage : public MapScene
{
public:
    SimpleStage();
    SimpleStage(const wstring& inMapPath);
    ~SimpleStage();
    
    void SetMapFilePath(const wstring& path);
    
protected:
    void Init() override;
    void OnLoadMap() override;
    
    void Enter() override;
};
