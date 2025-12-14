#include "pch.h"
#include "ScenePlayerDeath.h"

#include "Game/Enum.h"
#include "Game/Manager/EventBusManager.h"
#include "Game/Manager/SaveManager.h"
#include "Game/Manager/SFXManager.h"
#include "Game/UI/OverlayUI.h"

void ScenePlayerDeath::Init()
{
    deathUI = new OverlayUI(L"Image/Background/player_death_screen.bmp");
    AddUI(deathUI);
}

void ScenePlayerDeath::Enter()
{
    SFX->StopAll();
    SFX->PlayOnce(SFXKey::GameOver);
    CAMERA->FadeIn(1.5f);
}

void ScenePlayerDeath::Update()
{
    // Space: 체크포인트로 리스폰
    if (INPUT->ButtonDown(VK_SPACE))
    {
        auto checkpoint = SAVE->GetLastCheckpoint();
        if (checkpoint.IsValid())
        {
            // 모든 게임 씬 리셋 예약 (적, 오브젝트 리스폰)
            WORLD->ResetAllScenes();

            // Player 제거 (새로 생성되도록)
            DeletePersistentObjects();

            CAMERA->FadeOut(0.5f);
            WORLD->ChangeScene(checkpoint.sceneType, 0.5f);
        }
        else
        {
            // 체크포인트가 없으면 타이틀로
            CAMERA->FadeOut(1.5f);
            WORLD->ChangeScene((int)ESceneType::Title, 1.5f);
        }
    }
    // ESC: 메인 메뉴(타이틀)로
    else if (INPUT->ButtonDown(VK_ESCAPE))
    {
        CAMERA->FadeOut(1.5f);
        WORLD->ChangeScene((int)ESceneType::Title, 1.5f);
    }
}

void ScenePlayerDeath::Render()
{
}

void ScenePlayerDeath::Exit()
{
}

void ScenePlayerDeath::Release()
{
}
