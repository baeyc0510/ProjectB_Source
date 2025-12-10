#include "pch.h"
#include "Stage_Boss01.h"

#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/SFXManager.h"

Stage_Boss01::Stage_Boss01()
{
    mapFilePath = TEXT("Maps/stage_boss.json");
}

void Stage_Boss01::OnLoadMap()
{
    MapScene::OnLoadMap();
    SpawnPlayer();
    GAMEUI->ShowPlayerHUD(true);
}

void Stage_Boss01::Enter()
{
    MapScene::Enter();
}

void Stage_Boss01::Exit()
{
    MapScene::Exit();
    SFX->Stop(SFXKey::BGM_Piedad);
}
