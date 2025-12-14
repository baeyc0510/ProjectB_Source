#pragma once
#include "SimpleStage.h"

class Stage_Brotherhood : public SimpleStage
{
public:
    Stage_Brotherhood();
    Stage_Brotherhood(const wstring& inMapPath);
    ~Stage_Brotherhood() override = default;
    
protected:
    /*~ Scene Interface ~*/
    void Enter() override;
};
