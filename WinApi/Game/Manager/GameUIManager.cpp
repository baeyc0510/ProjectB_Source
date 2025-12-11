#include "pch.h"
#include "GameUIManager.h"

#include "Game/UI/BossHUD.h"
#include "Game/UI/PlayerStatusHUD.h"
#include "Game/UI/InventoryUI.h"

GameUIManager::GameUIManager()
	: statusHUD(nullptr)
{
}

GameUIManager::~GameUIManager()
{
}

void GameUIManager::Init()
{
	statusHUD = new PlayerStatusHUD();
	statusHUD->SetPos(Vec2(STATUS_HUD_X, STATUS_HUD_Y));
	statusHUD->SetScreenFixed(true);
	statusHUD->SetVisibility(false);
	AddUI(statusHUD);
	
	bossHUD = new BossHUD();
	bossHUD->SetPos(Vec2(BOSS_HUD_X, BOSS_HUD_Y));
	bossHUD->SetScreenFixed(true);
	bossHUD->SetVisibility(false);
	AddUI(bossHUD);
}

void GameUIManager::Update()
{
	// I: 인벤토리 토글
	if (INPUT->ButtonDown('I'))
	{
		ToggleUI(EOverlayUI::Inventory);
	}

	for (UIBase* ui : uiList)
	{
		ui->ComponentUpdate();
	}
}

void GameUIManager::Render()
{
	RENDER->BeginUI();
	for (UIBase* ui : uiList)
	{
		ui->ComponentRender();
	}
	RENDER->EndUI();
}

bool GameUIManager::ConsumeEscapeInput()
{
	if (INPUT->ButtonDown(VK_ESCAPE) && IsUIOpen())
	{
		CloseUI();
		return true;
	}
	return false;
}

void GameUIManager::Release()
{
	for (UIBase* ui : uiList)
	{
		ui->ComponentRelease();
		delete ui;
	}
	uiList.clear();
	statusHUD = nullptr;

	while (!overlayStack.empty())
		overlayStack.pop();
}

void GameUIManager::SetPlayerHP(float current, float max)
{
	if (statusHUD)
		statusHUD->SetHP(current, max);
}

void GameUIManager::SetPlayerMP(float current, float max)
{
	if (statusHUD)
		statusHUD->SetMP(current, max);
}

void GameUIManager::SetPlayerFlask(int current, int max)
{
	if (statusHUD)
		statusHUD->SetFlask(current, max);
}

void GameUIManager::SetBossHP(float current, float max)
{
	if (bossHUD)
		bossHUD->SetHP(current, max);
}

void GameUIManager::SetBossName(ImageResource* nameImg)
{
	if (bossHUD)
		bossHUD->SetBossNameImage(nameImg);
}

void GameUIManager::ShowPlayerHUD(bool show)
{
	if (statusHUD)
		statusHUD->SetVisibility(show);
}

void GameUIManager::ShowBossHUD(bool show)
{
	if (bossHUD)
		bossHUD->SetVisibility(show);
}

void GameUIManager::CloseHUD()
{
	ShowPlayerHUD(false);
	ShowBossHUD(false);
}

void GameUIManager::AddUI(UIBase* ui)
{
	uiList.push_back(ui);
	ui->ComponentInit();
	ui->ComponentOnEnable();
}

void GameUIManager::DeleteUI(UIBase* ui)
{
	ui->ComponentOnDisable();
	ui->ComponentRelease();
	uiList.remove(ui);
	delete ui;
}

void GameUIManager::OpenUI(EOverlayUI type)
{
	UIBase* overlay = CreateOverlay(type);
	if (overlay)
	{
		overlay->SetScreenFixed(true);
		AddUI(overlay);
		overlayStack.push({ type, overlay });
	}
}

void GameUIManager::CloseUI()
{
	if (overlayStack.empty())
		return;

	auto& top = overlayStack.top();
	UIBase* overlay = top.second;
	DeleteUI(overlay);
	overlayStack.pop();
}

void GameUIManager::ToggleUI(EOverlayUI type)
{
	if (GetCurrentUI() == type)
		CloseUI();
	else
		OpenUI(type);
}

EOverlayUI GameUIManager::GetCurrentUI() const
{
	if (overlayStack.empty())
		return EOverlayUI::None;
	return overlayStack.top().first;
}

UIBase* GameUIManager::CreateOverlay(EOverlayUI type)
{
	switch (type)
	{
	case EOverlayUI::Inventory:
		return new InventoryUI();
	case EOverlayUI::BossDefeat:
		return new OverlayUI(L"Image/Background/boss-defeated-screen-title.bmp");
	default:
		return nullptr;
	}
}
