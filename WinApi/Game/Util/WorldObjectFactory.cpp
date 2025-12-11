#include "pch.h"
#include "WorldObjectFactory.h"
#include "Game/Object/Character/Boss_TenPiedad.h"
#include "Game/Object/Character/Enemy_Acolyte.h"
#include "Game/Object/Character/Enemy_Stoner.h"
#include "Game/Object/World/Checkpoint.h"

GameObject* WorldObjectFactory::CreateWorldObject(const string& objectType)
{
    GameObject* object = nullptr;
    
    // TODO: object 타입 추가
    if (objectType == "Acolyte") object = new Enemy_Acolyte();
    if (objectType == "Stoner") object = new Enemy_Stoner();
    if (objectType == "Piedad") object = new Boss_TenPiedad();
    if (objectType == "CheckPoint") object = new Checkpoint();
    
    assert(object && TEXT("Invalid object Type!"));
    return object;
}
