#include "pch.h"
#include "CMapScene.h"

#include "Game/CGame.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CMapManager.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Object/World/CTransitionArea.h"

CMapScene::CMapScene()
{
    spawnId = 0;
}

CMapScene::~CMapScene()
{
}

CPlayer* CMapScene::SpawnPlayer()
{
    // 씬에 이미 있는 Player 검색
    CPlayer* player = FindObjectByType<CPlayer>();

    if (!player)
    {
        player = new CPlayer();
        AddGameObject(player);
    }

    player->SetPos(MAP->GetPlayerSpawn(spawnId));

    return player;
}

void CMapScene::Init()
{
    MAP->LoadMap(mapFilePath);
    
    // TrasitionArea 배치
    SpawnTransitionArea();
    
    // 월드 콜라이더 생성
    MAP->CreateWorldColliders(this);
    
    // 월드 오브젝트들 생성 (적 캐릭터)
    MAP->CreateWorldCharacters(this);
}

void CMapScene::Enter()
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

    CPlayer* player = FindObjectByType<CPlayer>();
    if (player)
    {
        CAMERA->SetTargetObj(player);
    }
}

void CMapScene::Update()
{
    // TEMP
    // ESC: UI가 처리 안 했으면 씬 전환
    if (!GAMEUI->ConsumeEscapeInput() && INPUT->ButtonDown(VK_ESCAPE))
    {
        CAMERA->FadeOut(0.5f);
        WORLD->ChangeScene(ESceneType::Title, 0.5f);
    }
}

void CMapScene::Render()
{
}

void CMapScene::Exit()
{
}

void CMapScene::Release()
{
}

void CMapScene::RenderBackground()
{
    MAP->RenderBackground();
}

void CMapScene::RenderForeground()
{
    MAP->RenderForeground();
}

void CMapScene::SpawnTransitionArea()
{
    const vector<SceneTransitionData>& transitions = MAP->GetTransitions();
    for (const SceneTransitionData& transition : transitions)
    {
        CTransitionArea* area = new CTransitionArea();
        area->InitArea(transition);
        AddGameObject(area);
    }
}

void CMapScene::OnLoadMap()
{
    
    
}
