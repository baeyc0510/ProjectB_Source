#pragma once
#include "Game/Enum.h"
#include <stack>

class CScene;
class CUI;
class CPlayerStatusHUD;

class CGameUIManager : public SingleTon<CGameUIManager>
{
	friend SingleTon<CGameUIManager>;

private:
	CGameUIManager();
	~CGameUIManager();

public:
	void Init(CScene* scene);
	void Update();
	void Release();

	// Player 상태 업데이트
	void SetPlayerHP(float current, float max);
	void SetPlayerMP(float current, float max);
	void SetPlayerFlask(int current, int max);
	
	// Overlay UI 관리
	void OpenUI(EOverlayUI type);
	void CloseUI();
	void ToggleUI(EOverlayUI type);
	bool IsUIOpen() const { return !overlayStack.empty(); }
	EOverlayUI GetCurrentUI() const;

	// ESC로 UI 닫혔는지 (Scene에서 씬 전환 판단용)
	bool ConsumeEscapeInput();

private:
	CUI* CreateOverlay(EOverlayUI type);

private:
	CScene* currentScene;
	CPlayerStatusHUD* statusHUD;
	std::stack<std::pair<EOverlayUI, CUI*>> overlayStack;

	static constexpr float STATUS_HUD_X = 20.f;
	static constexpr float STATUS_HUD_Y = 20.f;
};

#define GAMEUI	CGameUIManager::GetInstance()
