#pragma once
#include "Enum.h"

class CGame
{
public:
	CGame();
	virtual ~CGame();

public:
	static const Vec2		WINSTART;
	static const Vec2		WINSIZE;
	static const Vec2		VIRTUALSIZE;	// 가상 해상도 (게임 로직 기준)
	static const DWORD		WINSTYLE = WS_SYSMENU | WS_MINIMIZEBOX;

public:
	void			Init(HINSTANCE hInstance);
	void			Run();
	void			Release();

private:
	void			Input();
	void			Update();
	void			Render();

private:
	HINSTANCE		hInst;
	HWND			hWnd;
};
