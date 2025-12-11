#include "pch.h"
#include "CharacterFactory.h"
#include "Game/Object/Character/Boss_TenPiedad.h"
#include "Game/Object/Character/Enemy_Acolyte.h"
#include "Game/Object/Character/Enemy_Stoner.h"

Character* CharacterFactory::CreateCharacter(const string& characterType)
{
    Character* character = nullptr;
    
    // TODO: character 타입 추가
    if (characterType == "Acorite") character = new Enemy_Acolyte();
    if (characterType == "Stoner") character = new Enemy_Stoner();
    if (characterType == "Piedad") character = new Boss_TenPiedad();
    
    assert(character && TEXT("Invalid character Type!"));
    return character;
}
