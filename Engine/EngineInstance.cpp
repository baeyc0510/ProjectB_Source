#include "pch.h"

#include "EngineInstance.h"


EngineInstance::EngineInstance()
{
    hInst = 0;
    hWnd = 0;
    winSize = Vec2(0, 0);
    virtualSize = Vec2(0, 0);
    renderScale = 1.0f;
}


EngineInstance::~EngineInstance()
{
}


void EngineInstance::Init(HINSTANCE hInst, HWND hWnd, Vec2 winSize, Vec2 virtualSize)
{
    this->hInst = hInst;
    this->hWnd = hWnd;

    // 실제 클라이언트 영역 크기 사용
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    this->winSize = Vec2((float)clientRect.right, (float)clientRect.bottom);

    // 가상 해상도 설정 (지정하지 않으면 윈도우 크기와 동일)
    if (virtualSize.x <= 0 || virtualSize.y <= 0)
        this->virtualSize = this->winSize;
    else
        this->virtualSize = virtualSize;

    // 렌더 스케일 계산 (가상 해상도 → 실제 윈도우)
    this->renderScale = this->winSize.x / this->virtualSize.x;
}


void EngineInstance::Release()
{
}
