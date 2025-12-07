#pragma once
#include "Game/Ability/Ability.h"

class Ability_BossAppear : public Ability
{
public:
    void OnActivate() override;
    void OnEnd() override;
};
