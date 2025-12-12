#pragma once
#include "Enemy.h"

class Enemy_Acolyte : public Enemy
{
public:
    Enemy_Acolyte();
    
protected:
    /*~  GameObject Interface ~*/
    void Init() override;
    
    /*~ Combat Interface ~*/
    bool ShouldBlockEnemy() override {return true;}
private:
    // 기본 스탯
    static constexpr float MAX_HP = 300.f;
    static constexpr float ATTACK_POWER = 90.f;
};