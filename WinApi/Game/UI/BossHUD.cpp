#include "pch.h"
#include "BossHUD.h"

BossHUD::BossHUD()
{
    currentHP = 100.f;
    maxHP = 100.f;
    hudScale = 2.2f;
}

BossHUD::~BossHUD()
{
}

void BossHUD::SetVisibility(bool visibility)
{
    SetShow(visibility);
}

void BossHUD::SetHP(float current, float max)
{
    currentHP = current;
    maxHP = max;
}

void BossHUD::SetBossNameImage(ImageResource* image)
{
    imgName = image;
    if (image)
    {
        nameWidth = (float)image->GetBmpWidth();
        nameHeight = (float)image->GetBmpHeight();
    }
}

void BossHUD::Init()
{
    imgFrame = LOADIMAGE(L"Boss_HP_Frame", L"Image/Sheet/boss_healthBar.bmp");
    imgHPBar = LOADIMAGE(L"Boss_HP", L"Image/Sheet/boss_healthBlood.bmp");

    // Set scale to frame size
    if (imgFrame)
    {
        scale = Vec2((float)imgFrame->GetBmpWidth(), (float)imgFrame->GetBmpHeight());
    }
}

void BossHUD::OnEnable()
{
}

void BossHUD::Update()
{
}

void BossHUD::Render()
{
    if (!imgFrame || !imgHPBar)
        return;

    float s = hudScale;
    float baseX = renderPos.x;
    float baseY = renderPos.y;

    // HP bar
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

    // Frame
    float frameW = scale.x * s;
    float frameH = scale.y * s;
    RENDER->TransparentImage(imgFrame,
        baseX, baseY,
        baseX + frameW, baseY + frameH);

    // Boss name (프레임 상단 가운데)
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

void BossHUD::OnDisable()
{
}

void BossHUD::Release()
{
}
