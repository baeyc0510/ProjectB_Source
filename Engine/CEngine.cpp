#include "pch.h"

#include "CEngine.h"


CEngine::CEngine()
{
    hInst = 0;
    hWnd = 0;
    winSize = Vec2(0, 0);
}


CEngine::~CEngine()
{
}


void CEngine::Init(HINSTANCE hInst, HWND hWnd, Vec2 winSize)
{
    this->hInst = hInst;
    this->hWnd = hWnd;

    // 실제 클라이언트 영역 크기 사용
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    this->winSize = Vec2((float)clientRect.right, (float)clientRect.bottom);
}


void CEngine::Release()
{
}
