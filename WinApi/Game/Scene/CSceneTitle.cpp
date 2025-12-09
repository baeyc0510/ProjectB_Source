#include "pch.h"
#include "CSceneTitle.h"
#include "Game/VFXKeys.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Game/UI/Buttons/CButton_MainMenu.h"

CSceneTitle::CSceneTitle()
{
}

CSceneTitle::~CSceneTitle()
{
}

void CSceneTitle::Init()
{
	btnEnter = new CButton_MainMenu(TEXT("BtnEnter"), TEXT("Image/Sheet/button_starton.bmp"),TEXT("Image/Sheet/button_start.bmp"));
	btnEnter->SetPos(Vec2(TITLE_ENTER_X, TITLE_ENTER_Y));
	btnEnter->SetButtonScale(2.0f);
	btnEnter->SetScreenFixed(true);
	btnEnter->SetShow(false);
	btnEnter->onButtonHover.Bind([this]()
	{
		FocusEnterButton();
	});
	btnEnter->onButtonPressed.Bind([this]()
	{
		ShowMainMenu(false);
		CAMERA->FadeOut(0.5f);
		WORLD->ChangeScene(ESceneType::Stage_Boss01, 0.5f);
	});
	AddUI(btnEnter);
	
	btnExit = new CButton_MainMenu(TEXT("BtnExit"), TEXT("Image/Sheet/button_exit_on.bmp"),TEXT("Image/Sheet/button_exit.bmp"));
	btnExit->SetPos(Vec2(TITLE_EXIT_X, TITLE_EXIT_Y));
	btnExit->SetButtonScale(2.0f);
	btnExit->SetScreenFixed(true);
	btnExit->SetShow(false);
	btnExit->onButtonHover.Bind([this]()
	{
		FocusExitButton();
	});
	AddUI(btnExit);
}

void CSceneTitle::Enter()
{
	CAMERA->FadeIn(0.5f);
	
	// 배경 레이어
	if (!bgLayer)
	{
		bgLayer = VFX->CreateVFX(VFXKey::Title_Bg);
		bgLayer->SetLooping(true);
	}

	// 메인 레이어
	if (!mainLayer)
	{
		mainLayer = VFX->CreateVFX(VFXKey::Title_Main);
		mainLayer->SetLooping(true);
	}

	// 꽃잎 레이어
	if (!petalsLayer)
	{
		petalsLayer = VFX->CreateVFX(VFXKey::Title_Petals);
		petalsLayer->SetLooping(true);
	}

	// 전경 레이어
	if (!fgLayer)
	{
		fgLayer = VFX->CreateVFX(VFXKey::Title_Fg);
		fgLayer->SetLooping(true);
	}
	
	// 카메라 중앙에 오게 설정
	CAMERA->SetTargetObj(mainLayer);

	// 모든 애니메이션 재생
	bgLayer->PlayVFX();
	mainLayer->PlayVFX();
	petalsLayer->PlayVFX();
	fgLayer->PlayVFX();
	
	// BMG 재생
	SFX->PlayBGM(SFXKey::BGM_Title);
	
	// UI 표시
	ShowMainMenu(true);
}

void CSceneTitle::Update()
{
}

void CSceneTitle::Render()
{
}

void CSceneTitle::Exit()
{
}

void CSceneTitle::Release()
{
}

void CSceneTitle::ShowMainMenu(bool show)
{
	if (btnEnter)
		btnEnter->SetShow(show);
	if (btnExit)
		btnExit->SetShow(show);
}

void CSceneTitle::FocusEnterButton()
{
	if (btnEnter)
		btnEnter->SetButtonActive(true);
	if (btnExit)
		btnExit->SetButtonActive(false);
}

void CSceneTitle::FocusExitButton()
{
	if (btnEnter)
		btnEnter->SetButtonActive(false);
	if (btnExit)
		btnExit->SetButtonActive(true);
}
