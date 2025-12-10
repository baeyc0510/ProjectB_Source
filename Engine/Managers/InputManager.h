#pragma once
class InputManager : public SingleTon<InputManager>
{
	friend SingleTon<InputManager>;
private:
	InputManager();
	virtual ~InputManager();

	static const UINT VKEY_SIZE = 0xFF;	// VK의 최대 갯수

public:
	void					Init();
	void					Update();
	void					Release();

	bool					ButtonStay(const int key, bool ignoreUI = false);	// 키를 누르고 있는 중
	bool					ButtonUp(const int key, bool ignoreUI = false);		// 키가 올라간 순간
	bool					ButtonDown(const int key, bool ignoreUI = false);	// 키가 내려간 순간
	Vec2					MouseScreenPos();									// 마우스 화면 위치
	Vec2					MouseWorldPos();									// 마우스 게임 위치

private:
	HWND					hWnd;

	array<bool, VKEY_SIZE>	prevKeys;						// 이전 키의 입력상태
	array<bool, VKEY_SIZE>	curKeys;						// 현재 키의 입력상태
	POINT					mousePos;						// 현재 마우스 위치
};

#define	INPUT				InputManager::GetInstance()
