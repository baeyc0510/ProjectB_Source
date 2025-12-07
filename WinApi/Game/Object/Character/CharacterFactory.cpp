#include "pch.h"
#include "CharacterFactory.h"

#include "CEnemy_Acorite.h"
#include "CBoss_TenPiedad.h"

CEnemy* CharacterFactory::CreateEnemy(string enemyType)
{
    CEnemy* monster = nullptr;

    // TODO: Enemy 타입 추가
    if (enemyType == "Acorite") monster = new CEnemy_Acorite();

    assert(monster && TEXT("Invalid Enemy Type!"));
    return monster;
}

CBoss* CharacterFactory::CreateBoss(string bossType)
{
    CBoss* boss = nullptr;

    if (bossType == "TenPiedad") boss = new CBoss_TenPiedad();

    assert(boss && TEXT("Invalid Boss Type!"));
    return boss;
}