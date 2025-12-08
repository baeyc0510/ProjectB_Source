#include "pch.h"
#include "CBossHUD.h"

CBossHUD::CBossHUD()
{
    currentHP = 100.f;
    maxHP = 100.f;
    hudScale = 2.2f;
}

CBossHUD::~CBossHUD()
{
}

void CBossHUD::SetVisibility(bool visibility)
{
    SetShow(visibility);
}

void CBossHUD::SetHP(float current, float max)
{
    currentHP = current;
    maxHP = max;
}

void CBossHUD::SetBossNameImage(CImage* image)
{
    imgName = image;
    if (image)
    {
        nameWidth = (float)image->GetBmpWidth();
        nameHeight = (float)image->GetBmpHeight();
    }
}

void CBossHUD::Init()
{
    imgFrame = LOADIMAGE(L"Boss_HP_Frame", L"Image/Sheet/boss_healthBar.bmp");
    imgHPBar = LOADIMAGE(L"Boss_HP", L"Image/Sheet/boss_healthBlood.bmp");

    // Set scale to frame size
    if (imgFrame)
    {
        scale = Vec2((float)imgFrame->GetBmpWidth(), (float)imgFrame->GetBmpHeight());
    }
}

void CBossHUD::OnEnable()
{
}

void CBossHUD::Update()
{
}

void CBossHUD::Render()
{
    if (!imgFrame || !imgHPBar)
        return;

    float s = hudScale;
    float baseX = renderPos.x;
    float baseY = renderPos.y;

    // 1. HP bar
    float hpRatio = (maxHP > 0.f) ? (currentHP / maxHP) : 0.f;
    float hpSrcWidth = (float)imgHPBar->GetBmpWidth() * hpRatio;
    float hpSrcHeight = (float)imgHPBar->GetBmpHeight();
    float hpDstWidth = hpSrcWidth * s;
    float hpDstHeight = hpSrcHeight * s;

    float hpX = baseX + HP_BAR_OFFSET_X * s;
    float hpY = baseY + HP_BAR_OFFSET_Y * s;

    RENDER->FrameImage(imgHPBar,
        hpX, hpY, hpX + hpDstWidth, hpY + hpDstHeight,
        0, 0, hpSrcWidth, hpSrcHeight,
        false);

    // 2. frame
    float frameW = scale.x * s;
    float frameH = scale.y * s;
    RENDER->TransparentImage(imgFrame,
        baseX, baseY,
        baseX + frameW, baseY + frameH);

    // 3. Boss name (프레임 상단 가운데)
    if (imgName)
    {
        float nameDstW = nameWidth * s;
        float nameDstH = nameHeight * s;
        float nameX = baseX + (frameW - nameDstW) * 0.5f;
        float nameY = baseY + NAME_OFFSET_Y;

        RENDER->TransparentImage(imgName,
            nameX, nameY,
            nameX + nameDstW, nameY + nameDstH);
    }
}

void CBossHUD::OnDisable()
{
}

void CBossHUD::Release()
{
}
