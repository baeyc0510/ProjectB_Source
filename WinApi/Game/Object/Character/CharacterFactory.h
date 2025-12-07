#pragma once

class CEnemy;
class CBoss;

class CharacterFactory
{
public:
    static CEnemy* CreateEnemy(string enemyType);
    static CBoss* CreateBoss(string bossType);
};