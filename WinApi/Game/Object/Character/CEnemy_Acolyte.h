#pragma once
#include "CEnemy.h"

class CEnemy_Acolyte : public CEnemy
{
public:
    CEnemy_Acolyte();
    
protected:
    void Init() override;
};
