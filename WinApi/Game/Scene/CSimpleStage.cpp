#include "pch.h"
#include "CSimpleStage.h"

#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CSFXManager.h"

CSimpleStage::CSimpleStage()
{
}

CSimpleStage::CSimpleStage(const wstring& inMapPath)
{
    mapFilePath = inMapPath;
}

CSimpleStage::~CSimpleStage()
{
}

void CSimpleStage::SetMapFilePath(const wstring& path)
{
    mapFilePath = path;
}

void CSimpleStage::Init()
{
    CMapScene::Init();
}

void CSimpleStage::OnLoadMap()
{
    CMapScene::OnLoadMap();
    
    GAMEUI->ShowPlayerHUD(true);
    // Player (맵의 스폰 위치 사용)
    SpawnPlayer();
}

void CSimpleStage::Enter()
{
    CMapScene::Enter();
    
    // SFX
    SFX->PlayOnce(SFXKey::MapEnter);
}
