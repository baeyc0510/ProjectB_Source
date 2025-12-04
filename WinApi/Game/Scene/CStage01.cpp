#include "pch.h"
#include "CStage01.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CEnemy.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CMapManager.h"
#include "Game/Object/Character/CEnemy_Acorite.h"

class CCameraController;
class CPlayer;

CStage01::CStage01() : _player(nullptr), _stateSystem(nullptr)
{
	mapFilePath = TEXT("Maps/stage01.json");
}

CStage01::~CStage01()
{
}

void CStage01::Init()
{
	CMapScene::Init();
}

void CStage01::Enter()
{
	CMapScene::Enter();
}

void CStage01::Update()
{
	CMapScene::Update();
}

void CStage01::Render()
{
	CMapScene::Render();
	// // // 디버그: 플레이어 state 출력
	// wstring debugPlayer = _stateSystem->GetStateTagString();
	// RENDER->Text(40, 55, debugPlayer);
}

void CStage01::Exit()
{
	CMapScene::Exit();
}

void CStage01::Release()
{
	CMapScene::Release();
}

void CStage01::OnLoadMap()
{
	CMapScene::OnLoadMap();
	
	GAMEUI->ShowHUD(true);
	// Player (맵의 스폰 위치 사용)
	_player = SpawnPlayer();
	_stateSystem = _player->GetComponent<CStateSystem>();
}
