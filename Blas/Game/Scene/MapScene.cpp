#include "pch.h"
#include "MapScene.h"

#include "Game/Component/StatComponent.h"
#include "Game/Enum.h"
#include "Game/Game.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/MapManager.h"
#include "Game/Manager/SaveManager.h"
#include "Game/Object/Character/Boss.h"
#include "Game/Object/Character/Enemy.h"
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

    Vec2 spawnPos;
    auto checkpoint = SAVE->GetLastCheckpoint();

    // 체크포인트 리스폰 체크
    if (checkpoint.IsValid() &&
        checkpoint.sceneType == SINGLE(SceneManager)->GetCurSceneKey())
    {
        // 체크포인트 위치에서 스폰
        spawnPos = checkpoint.position;

        // 리소스 완전 회복
        StatComponent* stat = player->GetStatComponent();
        if (stat)
        {
            stat->SetCurrent(EStatType::HP, stat->GetMax(EStatType::HP));
            stat->SetCurrent(EStatType::MP, stat->GetMax(EStatType::MP));
            stat->SetCurrent(EStatType::Flask, stat->GetMax(EStatType::Flask));
        }
    }
    else
    {
        // 일반 스폰 (spawnId 사용)
        spawnPos = MAP->GetPlayerSpawn(spawnId);
    }

    player->SetPos(spawnPos);

    return player;
}

void MapScene::LoadMap()
{
    assert(!mapFilePath.empty());
    MAP->LoadMap(mapFilePath);
}

void MapScene::Init()
{
    MAP->LoadMap(mapFilePath);
    MAP->SetActiveMap(mapFilePath);

    // TrasitionArea 배치
    SpawnTransitionArea();

    // 월드 콜라이더 생성
    MAP->CreateWorldColliders(this);

    // 월드 오브젝트들 생성 (적 캐릭터)
    MAP->CreateWorldCharacters(this);

    // 체크포인트 생성
    MAP->CreateWorldCheckpoints(this);
}

void MapScene::Enter()
{
    // Map 로드 및 활성화
    LoadMap();
    MAP->SetActiveMap(mapFilePath);

    OnLoadMap();
    
    // 카메라 세팅
    CAMERA->SetOffset(Game::DEFAULT_CAMERA_OFFSET);
    CAMERA->SetDeadZone(Game::DEFAULT_CAMERA_DEADZONE);
    CAMERA->SetSmoothSpeed(Game::DEFAULT_CAMERA_SMOOTH);
    CAMERA->SetBounds(MAP->GetBounds());

    Player* player = FindObjectByType<Player>();
    if (player)
    {
        CAMERA->SetTargetObj(player, true);
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

void MapScene::Reset()
{
	if (mapFilePath.empty())
		return;

	// 적만 삭제 후 재생성
	DeleteObjectsByType<Enemy>();
	DeleteObjectsByType<Boss>();

	// 맵 로드 (스폰 데이터용)
	LoadMap();

	// 적 재생성
	MAP->CreateWorldCharacters(this);
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
