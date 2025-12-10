#include "pch.h"
#include "COverlay_BossDefeat.h"

void COverlay_BossDefeat::Init()
{
    imgBackground = LOADIMAGE(L"Boss_Defeat", L"Image/Background/boss-defeated-screen-title.bmp");
    SetFullscreen(imgBackground);
}

void COverlay_BossDefeat::OnEnable()
{
    COverlayUI::OnEnable();
}

void COverlay_BossDefeat::Update()
{
    COverlayUI::Update();
}

void COverlay_BossDefeat::OnDisable()
{
    COverlayUI::OnDisable();
}

void COverlay_BossDefeat::Release()
{
    COverlayUI::Release();
}

void COverlay_BossDefeat::Render()
{
    if (!imgBackground)
        return;
	
    RENDER->TransparentImage(imgBackground,
        renderPos.x, renderPos.y,
        renderPos.x + scale.x, renderPos.y + scale.y);
}