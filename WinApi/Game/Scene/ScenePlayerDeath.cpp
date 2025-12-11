#include "pch.h"
#include "ScenePlayerDeath.h"

#include "Game/Enum.h"
#include "Game/Manager/EventBusManager.h"
#include "Game/UI/OverlayUI.h"

void ScenePlayerDeath::Init()
{
    deathUI = new OverlayUI(L"Image/Background/player_death_screen.bmp");
    AddUI(deathUI);
}

void ScenePlayerDeath::Enter()
{
    CAMERA->FadeIn(1.5f);
}

void ScenePlayerDeath::Update()
{
    if (INPUT->AnyButtonDown())
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
