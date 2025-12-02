#include "pch.h"
#include "CGameUIManager.h"
#include "Game/UI/CPlayerStatusHUD.h"
#include "Game/UI/CInventoryUI.h"

CGameUIManager::CGameUIManager()
	: statusHUD(nullptr)
	, hudVisible(true)
{
}

CGameUIManager::~CGameUIManager()
{
}

void CGameUIManager::Init()
{
	statusHUD = new CPlayerStatusHUD();
	statusHUD->SetPos(Vec2(STATUS_HUD_X, STATUS_HUD_Y));
	statusHUD->SetScreenFixed(true);
	AddUI(statusHUD);
	statusHUD->SetVisibility(false);
}

void CGameUIManager::Update()
{
	// I: 인벤토리 토글
	if (INPUT->ButtonDown('I'))
	{
		ToggleUI(EOverlayUI::Inventory);
	}

	for (CUI* ui : uiList)
	{
		ui->ComponentUpdate();
	}
}

void CGameUIManager::Render()
{
	RENDER->BeginUI();
	for (CUI* ui : uiList)
	{
		ui->ComponentRender();
	}
	RENDER->EndUI();
}

bool CGameUIManager::ConsumeEscapeInput()
{
	if (INPUT->ButtonDown(VK_ESCAPE) && IsUIOpen())
	{
		CloseUI();
		return true;
	}
	return false;
}

void CGameUIManager::Release()
{
	for (CUI* ui : uiList)
	{
		ui->ComponentRelease();
		delete ui;
	}
	uiList.clear();
	statusHUD = nullptr;

	while (!overlayStack.empty())
		overlayStack.pop();
}

void CGameUIManager::SetPlayerHP(float current, float max)
{
	if (statusHUD)
		statusHUD->SetHP(current, max);
}

void CGameUIManager::SetPlayerMP(float current, float max)
{
	if (statusHUD)
		statusHUD->SetMP(current, max);
}

void CGameUIManager::SetPlayerFlask(int current, int max)
{
	if (statusHUD)
		statusHUD->SetFlask(current, max);
}

void CGameUIManager::ShowHUD(bool show)
{
	hudVisible = show;
	if (statusHUD)
		statusHUD->SetVisibility(show);
}

void CGameUIManager::AddUI(CUI* ui)
{
	uiList.push_back(ui);
	ui->ComponentInit();
	ui->ComponentOnEnable();
}

void CGameUIManager::DeleteUI(CUI* ui)
{
	ui->ComponentOnDisable();
	ui->ComponentRelease();
	uiList.remove(ui);
	delete ui;
}

void CGameUIManager::OpenUI(EOverlayUI type)
{
	CUI* overlay = CreateOverlay(type);
	if (overlay)
	{
		overlay->SetScreenFixed(true);
		AddUI(overlay);
		overlayStack.push({ type, overlay });
	}
}

void CGameUIManager::CloseUI()
{
	if (overlayStack.empty())
		return;

	auto& top = overlayStack.top();
	CUI* overlay = top.second;
	DeleteUI(overlay);
	overlayStack.pop();
}

void CGameUIManager::ToggleUI(EOverlayUI type)
{
	if (GetCurrentUI() == type)
		CloseUI();
	else
		OpenUI(type);
}

EOverlayUI CGameUIManager::GetCurrentUI() const
{
	if (overlayStack.empty())
		return EOverlayUI::None;
	return overlayStack.top().first;
}

CUI* CGameUIManager::CreateOverlay(EOverlayUI type)
{
	switch (type)
	{
	case EOverlayUI::Inventory:
		return new CInventoryUI();
	default:
		return nullptr;
	}
}
