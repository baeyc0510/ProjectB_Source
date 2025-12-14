#pragma once
#include "Enemy.h"

class Enemy_Stoner : public Enemy
{
public:
    /*~ GameObject Interface ~*/
    void Init() override;

protected:
    /*~ Enemy Interface ~*/
    void UpdateAIActions() override;
    void UpdateAnimation() override;
    void MoveInDirection(int dir, float speed) override;
    void OnStateChanged(EStateTag oldTags, EStateTag newTags) override;
    
private:
    void OnEndAggroMotion();
    void OnTurnaroundComplete();
    
private:
    // 기본 스탯
    static constexpr float MAX_HP = 300.f;
    static constexpr float ATTACK_POWER = 40.f;
    
    bool bIsInAggroMotion = false;
};
