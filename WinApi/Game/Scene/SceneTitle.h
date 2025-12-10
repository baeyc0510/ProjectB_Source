#pragma once

class Button_MainMenu;
class VFXObject;
class Animator;

class SceneTitle : public Scene
{
public:
	SceneTitle();
	virtual ~SceneTitle();

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
	VFXObject* bgLayer = nullptr;
	VFXObject* mainLayer = nullptr;
	VFXObject* petalsLayer = nullptr;
	VFXObject* fgLayer = nullptr;
	
	Button_MainMenu* btnEnter;
	Button_MainMenu* btnExit;
};