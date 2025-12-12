#include "pch.h"
#include "Stage_Beginning.h"

#include "Game/Game.h"
#include "Game/Data/AnimKey.h"
#include "Game/Object/Character/Player.h"
#include "Game/Component/StateSystem.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/SFXManager.h"

class CCameraController;
class Player;

Stage_Beginning::Stage_Beginning() : _player(nullptr), _stateSystem(nullptr)
{
	mapFilePath = TEXT("Maps/stage01.json");
}

Stage_Beginning::~Stage_Beginning()
{
}

void Stage_Beginning::Init()
{
	MapScene::Init();
}

void Stage_Beginning::Enter()
{
	MapScene::Enter();
	SFX->PlayBGM(SFXKey::BGM_Brotherhood);
	SFX->PlayAmbient(SFXKey::Ambient_Brotherhood);
	
	// 시작 씬 연출
	if (spawnId == 0)
	{
		Player* player = FindObjectByType<Player>();
		if (!player)
			return;
		
		Animator* animator = player->GetComponent<Animator>();
		// 누워있는 장면에서 멈춤
		animator->Play(AnimKey::Rising);
		animator->Stop();
		player->SetIsDown(true);
		
		CAMERA->SetLookAt(player->GetPos() - Vec2(0.f, 300.f));
		CAMERA->SetTargetPos(player->GetPos() + Game::DEFAULT_CAMERA_OFFSET , 2.0f);
		CAMERA->SetTargetObj(nullptr);
		
		// 2.5초후 일어나기 시작
		TIMER->SetTimer([this,player, animator]()
		{
			CAMERA->SetTargetObj(player);
			
			animator->Play(AnimKey::Rising,true,[this,player]()
			{
				// SFX
				SFX->PlayOnce(SFXKey::MapEnter);	
				
				TIMER->SetTimer([this,player]()
				{
					player->SetIsDown(false);
				},1.0f);
			});
		}, 2.5f);
	}
}

void Stage_Beginning::Update()
{
	MapScene::Update();
}

void Stage_Beginning::Render()
{
	MapScene::Render();
	// // // 디버그: 플레이어 state 출력
	// wstring debugPlayer = _stateSystem->GetStateTagString();
	// RENDER->Text(40, 55, debugPlayer);
}

void Stage_Beginning::Exit()
{
	MapScene::Exit();
}

void Stage_Beginning::Release()
{
	MapScene::Release();
}

void Stage_Beginning::OnLoadMap()
{
	MapScene::OnLoadMap();
	
	GAMEUI->ShowPlayerHUD(true);
	// Player (맵의 스폰 위치 사용)
	_player = SpawnPlayer();
	_stateSystem = _player->GetComponent<StateSystem>();
}
