#include "pch.h"
#include "CStage02.h"

#include "Game/Manager/CGameUIManager.h"

CStage02::CStage02()
{
    mapFilePath = TEXT("Maps/stage02.json");
}

CStage02::~CStage02()
{
}

void CStage02::Init()
{
    CMapScene::Init();
}

void CStage02::OnLoadMap()
{
    CMapScene::OnLoadMap();
    
    GAMEUI->ShowHUD(true);
    // Player (맵의 스폰 위치 사용)
    SpawnPlayer();
}
