#pragma once
#include "CMapScene.h"

class CStage_Boss01 : public CMapScene
{
public:
    CStage_Boss01();
    
protected:
    void OnLoadMap() override;
    void Enter() override;
    void Exit() override;
};