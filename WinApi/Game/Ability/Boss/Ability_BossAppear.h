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
    class CBoss* boss = nullptr;
    class CBossAIController* bossAI = nullptr;
    class CPlayer* player = nullptr;
};
