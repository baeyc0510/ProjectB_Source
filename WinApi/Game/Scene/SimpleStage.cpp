#include "pch.h"
#include "SimpleStage.h"

#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/SFXManager.h"

SimpleStage::SimpleStage()
{
}

SimpleStage::SimpleStage(const wstring& inMapPath)
{
    mapFilePath = inMapPath;
}

SimpleStage::~SimpleStage()
{
}

void SimpleStage::SetMapFilePath(const wstring& path)
{
    mapFilePath = path;
}

void SimpleStage::Init()
{
    MapScene::Init();
}

void SimpleStage::OnLoadMap()
{
    MapScene::OnLoadMap();
    
    GAMEUI->ShowPlayerHUD(true);
    // Player (맵의 스폰 위치 사용)
    SpawnPlayer();
}

void SimpleStage::Enter()
{
    MapScene::Enter();
    
    // SFX
    SFX->PlayOnce(SFXKey::MapEnter);
}
