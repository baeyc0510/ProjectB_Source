#include "pch.h"
#include "Overlay_BossDefeat.h"

void Overlay_BossDefeat::Init()
{
    imgBackground = LOADIMAGE(L"Boss_Defeat", L"Image/Background/boss-defeated-screen-title.bmp");
    SetFullscreen(imgBackground);
}

void Overlay_BossDefeat::OnEnable()
{
    OverlayUI::OnEnable();
}

void Overlay_BossDefeat::Update()
{
    OverlayUI::Update();
}

void Overlay_BossDefeat::OnDisable()
{
    OverlayUI::OnDisable();
}

void Overlay_BossDefeat::Release()
{
    OverlayUI::Release();
}

void Overlay_BossDefeat::Render()
{
    if (!imgBackground)
        return;
	
    RENDER->TransparentImage(imgBackground,
        renderPos.x, renderPos.y,
        renderPos.x + scale.x, renderPos.y + scale.y);
}