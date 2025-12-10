#pragma once

class CButton_MainMenu;
class CVFX;
class CAnimator;

class CSceneTitle : public CScene
{
public:
	CSceneTitle();
	virtual ~CSceneTitle();

public:
	void Init()		override;
	void Enter()	override;
	void Update()	override;
	void Render()	override;
	void Exit()		override;
	void Release()	override;
	
	// 메인메뉴 버튼 표시
	void ShowMainMenu(bool show);
	void FocusEnterButton();
	void FocusExitButton();

private:
	static constexpr float TITLE_ENTER_X = 1000;
	static constexpr float TITLE_ENTER_Y = 450;
	static constexpr float TITLE_EXIT_X = 1001;
	static constexpr float TITLE_EXIT_Y = 500;
	
	// 타이틀 배경 레이어 오브젝트들
	CVFX* bgLayer = nullptr;
	CVFX* mainLayer = nullptr;
	CVFX* petalsLayer = nullptr;
	CVFX* fgLayer = nullptr;
	
	CButton_MainMenu* btnEnter;
	CButton_MainMenu* btnExit;
};