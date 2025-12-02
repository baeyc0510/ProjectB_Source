#pragma once
#include "CMapScene.h"

class CStage02 : public CMapScene
{
public:
    CStage02();
    ~CStage02();
    
protected:
    void Init() override;
    void OnLoadMap() override;
};
