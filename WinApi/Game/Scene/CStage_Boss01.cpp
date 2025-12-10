#include "pch.h"
#include "CStage_Boss01.h"

#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CSFXManager.h"

CStage_Boss01::CStage_Boss01()
{
    mapFilePath = TEXT("Maps/stage_boss.json");
}

void CStage_Boss01::OnLoadMap()
{
    CMapScene::OnLoadMap();
    SpawnPlayer();
    GAMEUI->ShowPlayerHUD(true);
}

void CStage_Boss01::Enter()
{
    CMapScene::Enter();
}

void CStage_Boss01::Exit()
{
    CMapScene::Exit();
    SFX->Stop(SFXKey::BGM_Piedad);
}
