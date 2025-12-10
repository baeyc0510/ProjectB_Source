#pragma once
#include "MapScene.h"

class Stage_Boss01 : public MapScene
{
public:
    Stage_Boss01();
    
protected:
    void OnLoadMap() override;
    void Enter() override;
    void Exit() override;
};