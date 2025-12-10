#pragma once
#include "Game/Ability/Ability.h"

class Ability_BossAppear : public Ability
{
public:
    void OnActivate() override;
    void OnEnd() override;
    
private:
    void PlayWakeUpSound();
    void OnHitGround();
    
private:
    class Boss* boss = nullptr;
    class BossAIController* bossAI = nullptr;
    class Player* player = nullptr;
};
