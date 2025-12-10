#pragma once
#include "Enemy.h"

class Enemy_Acolyte : public Enemy
{
public:
    Enemy_Acolyte();
    
protected:
    void Init() override;
};
