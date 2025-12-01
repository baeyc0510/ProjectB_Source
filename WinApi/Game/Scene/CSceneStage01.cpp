#include "pch.h"
#include "CSceneStage01.h"

#include "Game/CGame.h"
#include "Game/Object/World/CGround.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Camera/CCameraController.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CMonster.h"
#include "Game/Sound/CSoundController.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CMapManager.h"

class CCameraController;
class CPlayer;

CSceneStage01::CSceneStage01()
{
}

CSceneStage01::~CSceneStage01()
{
}

void CSceneStage01::Init()
{
	// Game UI
	GAMEUI->Init(this);

	// Map 로드
	MAP->LoadMap(L"Maps/test.json");

	// Player (맵의 스폰 위치 사용)
	CPlayer* player = new CPlayer();
	player->SetPos(MAP->GetPlayerSpawn());
	AddGameObject(player);

	_player = player;

	// // Dummy Monster (Player 오른쪽)
	// CMonster* monster = new CMonster();
	// monster->SetPos(MAP->GetPlayerSpawn() + Vec2(200.f, 0.f));
	// monster->SetForward(-1);
	// AddGameObject(monster);
}

void CSceneStage01::Enter()
{
	// 카메라가 플레이어를 따라가도록 설정 (SceneEnter 이후에 호출해야 함)
	CAMERA->SetTargetObj(_player);
	CAMERA->SetOffset(Vec2(0.f,-100.f));
	CAMERA->SetDeadZone(Vec2(100.f,100.f));
	CAMERA->SetBounds(MAP->GetBounds());

	_stateSystem = _player->GetComponent<CStateSystem>();
}

void CSceneStage01::Update()
{
	GAMEUI->Update();

	// ESC: UI가 처리 안 했으면 씬 전환
	if (!GAMEUI->ConsumeEscapeInput() && INPUT->ButtonDown(VK_ESCAPE))
	{
		CAMERA->FadeOut(0.5f);
		WORLD->ChangeScene(SceneType::Title, 0.5f);
	}
}

void CSceneStage01::Render()
{
	// RENDER->SetText(10, RGB(255, 0, 0), TextAlign::Left);
	//
	// // // 디버그: 카메라 lookAt 위치 출력
	// // Vec2 camPos = CAMERA->GetLookAt();
	// // wstring debugCam = L"Camera: " + to_wstring((int)camPos.x) + L", " + to_wstring((int)camPos.y);
	// // RENDER->Text(10, 10, debugCam);
	// //
	// // // 디버그: 플레이어 pos 출력
	// wstring debugPlayer = _stateSystem->GetStateTagString();
	// RENDER->Text(40, 55, debugPlayer);
	//
	// // 디버그: 카메라 targetObj 확인
	// const CGameObject* camTarget = CAMERA->GetTargetObj();
	// wstring debugTarget = L"TargetObj: ";
	// if (camTarget == nullptr)
	// 	debugTarget += L"NULL";
	// else if (camTarget == _player)
	// 	debugTarget += L"Player (OK)";
	// else
	// 	debugTarget += L"OTHER";
	// RENDER->Text(10, 40, debugTarget);
	//
	// // 디버그: (0,0) 월드좌표가 화면 어디에 그려지는지
	// Vec2 originScreen = CAMERA->WorldToScreenPoint(Vec2(0, 0));
	// wstring debugOrigin = L"Origin(0,0)->Screen: " + to_wstring((int)originScreen.x) + L", " + to_wstring((int)originScreen.y);
	// RENDER->Text(10, 55, debugOrigin);
	//
	// RENDER->SetText();
}

void CSceneStage01::Exit()
{
}

void CSceneStage01::Release()
{
}

void CSceneStage01::RenderBackground()
{
	MAP->RenderBackground();
}

void CSceneStage01::RenderForeground()
{
	MAP->RenderForeground();
}
