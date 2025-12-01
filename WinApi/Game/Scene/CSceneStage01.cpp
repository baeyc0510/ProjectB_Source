#include "pch.h"
#include "CSceneStage01.h"

#include "Game/CGame.h"
#include "Game/Object/World/CGround.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Camera/CCameraController.h"
#include "Game/Object/Character/CMonster.h"
#include "Game/Sound/CSoundController.h"
#include "Game/Manager/CGameUIManager.h"

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
	// Ground
	CGround* pGround = new CGround();
	pGround->SetName(L"Ground");
	pGround->SetPos(Vec2(CGame::WINSIZE.x / 2.f, CGame::WINSIZE.y - 50.f));
	pGround->SetScale(Vec2(CGame::WINSIZE.x, 100.f));

	CCollider* pGroundCollider = new CCollider();
	pGroundCollider->SetScale(pGround->GetScale());
	pGroundCollider->SetLayer(Layer::Ground);
	pGround->AddChild(pGroundCollider);
	AddGameObject(pGround);

	// Player
	CPlayer* player = new CPlayer();
	player->SetPos(Vec2(CGame::WINSIZE.x * 0.5f, CGame::WINSIZE.y * 0.5f));
	AddGameObject(player);
	
	// Dummy Monster
	CMonster* monster = new CMonster();
	monster->SetPos(Vec2(CGame::WINSIZE.x * 0.5f, CGame::WINSIZE.y * 0.5f) + Vec2(200.f,0.f));
	monster->SetForward(-1);
	AddGameObject(monster);
	
	// CCameraController* controller = new CCameraController();
	// AddGameObject(controller);

	// CSoundController* sound = new CSoundController();
	// AddGameObject(sound);

	// Game UI
	GAMEUI->Init(this);
}

void CSceneStage01::Enter()
{
	CAMERA->FadeIn(0.5f);
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
	Vec2 startPos = CAMERA->WorldToScreenPoint(Vec2(0, 0));
	Vec2 endPos = CAMERA->WorldToScreenPoint(Vec2(CGame::WINSIZE.x, CGame::WINSIZE.y));
}

void CSceneStage01::Exit()
{
}

void CSceneStage01::Release()
{
}
