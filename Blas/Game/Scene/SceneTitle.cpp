#include "pch.h"
#include "SceneTitle.h"
#include "Game/Data/VFXKeys.h"
#include "Game/Enum.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/SaveManager.h"
#include "Game/Manager/SFXManager.h"
#include "Game/Manager/VFXManager.h"
#include "Game/Object/VFXObject.h"
#include "Game/UI/Buttons/Button_MainMenu.h"

SceneTitle::SceneTitle()
{
}

SceneTitle::~SceneTitle()
{
}

void SceneTitle::Init()
{
	btnEnter = new Button_MainMenu(TEXT("BtnEnter"), TEXT("Image/Sheet/button_starton.bmp"),TEXT("Image/Sheet/button_start.bmp"));
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

		// 체크포인트 확인하여 시작 씬 결정
		auto checkpoint = SAVE->GetLastCheckpoint();
		if (checkpoint.IsValid())
		{
			// 체크포인트에서 리스폰
			WORLD->ChangeScene(checkpoint.sceneType, 0.5f);
		}
		else
		{
			// 새 게임 시작
			WORLD->ChangeScene((int)ESceneType::Stage01, 0.5f);
		}

		SINGLE(UIManager)->SetFocusedUI(nullptr); // 포커스 해제해야 키입력 받음
	});
	AddUI(btnEnter);
	
	btnExit = new Button_MainMenu(TEXT("BtnExit"), TEXT("Image/Sheet/button_exit_on.bmp"),TEXT("Image/Sheet/button_exit.bmp"));
	btnExit->SetPos(Vec2(TITLE_EXIT_X, TITLE_EXIT_Y));
	btnExit->SetButtonScale(2.0f);
	btnExit->SetScreenFixed(true);
	btnExit->SetShow(false);
	btnExit->onButtonHover.Bind([this]()
	{
		FocusExitButton();
	});
	btnExit->onButtonPressed.Bind([]()
	{
		PostQuitMessage(0);
	});
	AddUI(btnExit);
}

void SceneTitle::Enter()
{
	static bool bFirstEnter = true;

	if (!bFirstEnter)
	{
		// 게임에서 돌아온 경우: 게임 상태 리셋
		WORLD->ResetAllScenes();
		DeletePersistentObjects();
		GAMEUI->CloseHUD();
	}
	bFirstEnter = false;

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
	CAMERA->SetTargetObj(mainLayer, true);

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

void SceneTitle::Update()
{
	// 방향키로 버튼 포커스 전환
	if (INPUT->ButtonDown(VK_UP))
	{
		FocusEnterButton();
	}
	else if (INPUT->ButtonDown(VK_DOWN))
	{
		FocusExitButton();
	}

	// 엔터키로 포커스된 버튼 실행
	if (INPUT->ButtonDown(VK_RETURN) && focusedButton)
	{
		focusedButton->onButtonPressed.Invoke();
	}
}

void SceneTitle::Render()
{
}

void SceneTitle::Exit()
{
}

void SceneTitle::Release()
{
}

void SceneTitle::ShowMainMenu(bool show)
{
	if (btnEnter)
		btnEnter->SetShow(show);
	if (btnExit)
		btnExit->SetShow(show);
}

void SceneTitle::FocusEnterButton()
{
	if (btnEnter)
		btnEnter->SetButtonActive(true);
	if (btnExit)
		btnExit->SetButtonActive(false);
	focusedButton = btnEnter;
}

void SceneTitle::FocusExitButton()
{
	if (btnEnter)
		btnEnter->SetButtonActive(false);
	if (btnExit)
		btnExit->SetButtonActive(true);
	focusedButton = btnExit;
}
