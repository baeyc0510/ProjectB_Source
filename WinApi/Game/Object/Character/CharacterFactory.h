#pragma once

class CEnemy;

class CharacterFactory
{
public:
    static CEnemy* CreateEnemy(string enemyType); 
};