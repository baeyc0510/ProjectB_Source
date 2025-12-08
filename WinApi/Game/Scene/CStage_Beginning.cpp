#include "pch.h"
#include "CStage_Beginning.h"

#include "Game/AnimKey.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CSFXManager.h"

class CCameraController;
class CPlayer;

CStage_Beginning::CStage_Beginning() : _player(nullptr), _stateSystem(nullptr)
{
	mapFilePath = TEXT("Maps/stage01.json");
}

CStage_Beginning::~CStage_Beginning()
{
}

void CStage_Beginning::Init()
{
	CMapScene::Init();
}

void CStage_Beginning::Enter()
{
	CMapScene::Enter();
	
	// 시작 씬 연출
	if (spawnId == 0)
	{
		CPlayer* player = FindObjectByType<CPlayer>();
		if (!player)
			return;
		
		CAnimator* animator = player->GetComponent<CAnimator>();
		// 누워있는 장면에서 멈춤
		animator->Play(AnimKey::Rising);
		animator->Stop();
		player->SetIsDown(true);
		
		// 2초후 일어나기 시작
		TIMER->SetTimer([this,player, animator]()
		{
			animator->Play(AnimKey::Rising,true,[this,player]()
			{
				// SFX
				SFX->PlayOnce(SFXKey::MapEnter);	
				
				TIMER->SetTimer([this,player]()
				{
					player->SetIsDown(false);
				},1.0f);
			});
		}, 2.0f);
	}
}

void CStage_Beginning::Update()
{
	CMapScene::Update();
}

void CStage_Beginning::Render()
{
	CMapScene::Render();
	// // // 디버그: 플레이어 state 출력
	// wstring debugPlayer = _stateSystem->GetStateTagString();
	// RENDER->Text(40, 55, debugPlayer);
}

void CStage_Beginning::Exit()
{
	CMapScene::Exit();
}

void CStage_Beginning::Release()
{
	CMapScene::Release();
}

void CStage_Beginning::OnLoadMap()
{
	CMapScene::OnLoadMap();
	
	GAMEUI->ShowPlayerHUD(true);
	// Player (맵의 스폰 위치 사용)
	_player = SpawnPlayer();
	_stateSystem = _player->GetComponent<CStateSystem>();
}
