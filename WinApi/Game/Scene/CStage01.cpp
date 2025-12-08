#include "pch.h"
#include "CStage01.h"

#include "Game/AnimKey.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CEnemy.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CMapManager.h"
#include "Game/Manager/CSFXManager.h"
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
	
	GAMEUI->ShowPlayerHUD(true);
	// Player (맵의 스폰 위치 사용)
	_player = SpawnPlayer();
	_stateSystem = _player->GetComponent<CStateSystem>();
}
