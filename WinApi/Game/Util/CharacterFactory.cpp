#include "pch.h"
#include "CharacterFactory.h"
#include "Game/Object/Character/CBoss_TenPiedad.h"
#include "Game/Object/Character/CEnemy_Acorite.h"


CCharacter* CharacterFactory::CreateCharacter(const string& characterType)
{
    CCharacter* character = nullptr;
    
    // TODO: character 타입 추가
    if (characterType == "Acorite") character = new CEnemy_Acorite();
    if (characterType == "Piedad") character = new CBoss_TenPiedad();
    
    assert(character && TEXT("Invalid character Type!"));
    return character;
}