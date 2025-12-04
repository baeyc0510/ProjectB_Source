#include "pch.h"
#include "CStage_Boss01.h"

#include "Game/Manager/CGameUIManager.h"

CStage_Boss01::CStage_Boss01()
{
    mapFilePath = TEXT("Maps/stage_boss.json");
}

void CStage_Boss01::OnLoadMap()
{
    CMapScene::OnLoadMap();
    SpawnPlayer();
    GAMEUI->ShowHUD(true);
}
