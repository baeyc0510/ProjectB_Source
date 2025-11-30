#include "pch.h"
#include "CGameUIManager.h"
#include "Game/UI/CPlayerStatusHUD.h"
#include "Game/UI/CInventoryUI.h"

CGameUIManager::CGameUIManager()
	: currentScene(nullptr)
	, statusHUD(nullptr)
{
}

CGameUIManager::~CGameUIManager()
{
}

void CGameUIManager::Init(CScene* scene)
{
	currentScene = scene;

	statusHUD = new CPlayerStatusHUD();
	statusHUD->SetPos(Vec2(STATUS_HUD_X, STATUS_HUD_Y));
	statusHUD->SetScreenFixed(true);
	scene->AddUI(statusHUD);
}

void CGameUIManager::Update()
{
	// I: 인벤토리 토글
	if (INPUT->ButtonDown('I'))
	{
		ToggleUI(EOverlayUI::Inventory);
	}
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
	currentScene = nullptr;
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

void CGameUIManager::OpenUI(EOverlayUI type)
{
	if (!currentScene)
		return;

	CUI* overlay = CreateOverlay(type);
	if (overlay)
	{
		overlay->SetScreenFixed(true);
		currentScene->AddUI(overlay);
		overlayStack.push({ type, overlay });
	}
}

void CGameUIManager::CloseUI()
{
	if (overlayStack.empty() || !currentScene)
		return;

	auto& top = overlayStack.top();
	CUI* overlay = top.second;
	WORLD->Delete(currentScene, overlay);
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
