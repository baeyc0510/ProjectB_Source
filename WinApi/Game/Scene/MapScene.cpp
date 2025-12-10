#include "pch.h"
#include "MapScene.h"

#include "Game/CGame.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/MapManager.h"
#include "Game/Object/Character/Player.h"
#include "Game/Object/World/TransitionArea.h"

MapScene::MapScene()
{
    spawnId = 0;
}

MapScene::~MapScene()
{
}

Player* MapScene::SpawnPlayer()
{
    // 씬에 이미 있는 Player 검색
    Player* player = FindObjectByType<Player>();

    if (!player)
    {
        player = new Player();
        AddGameObject(player);
    }

    player->SetPos(MAP->GetPlayerSpawn(spawnId));

    return player;
}

void MapScene::Init()
{
    MAP->LoadMap(mapFilePath);
    
    // TrasitionArea 배치
    SpawnTransitionArea();
    
    // 월드 콜라이더 생성
    MAP->CreateWorldColliders(this);
    
    // 월드 오브젝트들 생성 (적 캐릭터)
    MAP->CreateWorldCharacters(this);
}

void MapScene::Enter()
{
    // Map 로드
    assert(!mapFilePath.empty());
    MAP->LoadMap(mapFilePath);

    OnLoadMap();
    
    // 카메라 세팅
    CAMERA->SetOffset(CGame::DEFAULT_CAMERA_OFFSET);
    CAMERA->SetDeadZone(CGame::DEFAULT_CAMERA_DEADZONE);
    CAMERA->SetSmoothSpeed(CGame::DEFAULT_CAMERA_SMOOTH);
    CAMERA->SetBounds(MAP->GetBounds());

    Player* player = FindObjectByType<Player>();
    if (player)
    {
        CAMERA->SetTargetObj(player);
    }
}

void MapScene::Update()
{
}

void MapScene::Render()
{
}

void MapScene::Exit()
{
}

void MapScene::Release()
{
}

void MapScene::RenderBackground()
{
    MAP->RenderBackground();
}

void MapScene::RenderForeground()
{
    MAP->RenderForeground();
}

void MapScene::SpawnTransitionArea()
{
    const vector<SceneTransitionData>& transitions = MAP->GetTransitions();
    for (const SceneTransitionData& transition : transitions)
    {
        TransitionArea* area = new TransitionArea();
        area->InitArea(transition);
        AddGameObject(area);
    }
}

void MapScene::OnLoadMap()
{
    
    
}
