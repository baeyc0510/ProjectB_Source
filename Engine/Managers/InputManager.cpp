#include "pch.h"
#include "InputManager.h"

InputManager::InputManager()
{
	hWnd = 0;

	curKeys.fill(false);
	prevKeys.fill(false);
	mousePos = { 0, 0 };
}

InputManager::~InputManager()
{
}

void InputManager::Init()
{
	hWnd = SINGLE(EngineInstance)->GetHWnd();
}

void InputManager::Update()
{
	// 현재 선택(Focus)된 윈도우가 게임 윈도우인가를 확인
	if (hWnd != GetFocus())
	{
		// 윈도우가 선택(Focus)된 상태가 아닐 경우 키입력을 해제시킴
		for (int key = 0; key < VKEY_SIZE; key++)
		{
			prevKeys[key] = curKeys[key];
			curKeys[key] = false;
		}
		return;
	}

	// 모든 키 사이즈만큼 반복하며 입력상태를 확인
	for (int key = 0; key < VKEY_SIZE; key++)
	{
		prevKeys[key] = curKeys[key];
		if (GetAsyncKeyState(key) & 0x8000)
		{
			curKeys[key] = true;
		}
		else
		{
			curKeys[key] = false;
		}
	}

	// GetCursorPos() 윈도우에서 모니터 좌상단 기준 마우스의 좌표를 반환
	GetCursorPos(&mousePos);
	// 모니터 좌상단 기준 마우스 좌표를 게임 윈도우 기준 마우스 위치로 계산
	ScreenToClient(hWnd, &mousePos);
}

void InputManager::Release()
{
}

bool InputManager::ButtonStay(const int key, bool ignoreUI)
{
	if (SINGLE(UIManager)->GetFocusedUI() != nullptr && !ignoreUI)
		return false;

	// 키가 눌리고 있는 중
	return prevKeys[key] == true && curKeys[key] == true;
}

bool InputManager::ButtonUp(const int key, bool ignoreUI)
{
	if (SINGLE(UIManager)->GetFocusedUI() != nullptr && !ignoreUI)
		return false;

	// 키가 올라간 순간
	return prevKeys[key] == true && curKeys[key] == false;
}

bool InputManager::ButtonDown(const int key, bool ignoreUI)
{
	if (SINGLE(UIManager)->GetFocusedUI() != nullptr && !ignoreUI)
		return false;

	// 키가 내려간 순간
	return prevKeys[key] == false && curKeys[key] == true;
}

Vec2 InputManager::MouseScreenPos()
{
	return Vec2((float)mousePos.x, (float)mousePos.y);
}

Vec2 InputManager::MouseWorldPos()
{
	return CAMERA->ScreenToWorldPoint(Vec2((float)mousePos.x, (float)mousePos.y));
}

