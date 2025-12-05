#include "pch.h"
#include "CharacterFactory.h"

#include "CEnemy_Acorite.h"

CEnemy* CharacterFactory::CreateEnemy(string enemyType)
{
    CEnemy* monster = nullptr;
    
    // TODO: Enemy 타입 추가
    if (enemyType == "Acorite") monster = new CEnemy_Acorite();
    
    assert(TEXT("Invalid Enemy Type!"));
    return monster;
}