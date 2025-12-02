#include "pch.h"
#include "CSceneTitle.h"
#include "Game/CGame.h"
#include "Game/Manager/CGameUIManager.h"


CSceneTitle::CSceneTitle()
{
}

CSceneTitle::~CSceneTitle()
{
}

void CSceneTitle::Init()
{
}

void CSceneTitle::Enter()
{
	CAMERA->FadeIn(0.5f);
}

void CSceneTitle::Update()
{
	if (INPUT->ButtonDown(VK_SPACE))
	{
		CAMERA->FadeOut(0.5f);
		WORLD->ChangeScene(ESceneType::Stage02, 0.5f);
	}
}

void CSceneTitle::Render()
{
	RENDER->SetText(10);
	RENDER->Text(CGame::VIRTUALSIZE.x * 0.5f, CGame::VIRTUALSIZE.y * 0.5f, TEXT("press space to start"));
	RENDER->SetText();
}

void CSceneTitle::Exit()
{
}

void CSceneTitle::Release()
{
}
