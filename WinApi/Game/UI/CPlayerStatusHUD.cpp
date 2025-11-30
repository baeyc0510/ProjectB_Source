#include "pch.h"
#include "CPlayerStatusHUD.h"
#include "Asset/CImage.h"

CPlayerStatusHUD::CPlayerStatusHUD()
	: imgFrame(nullptr)
	, imgHPBar(nullptr)
	, imgMPBar(nullptr)
	, currentHP(100.f)
	, maxHP(100.f)
	, currentMP(100.f)
	, maxMP(100.f)
{
}

CPlayerStatusHUD::~CPlayerStatusHUD()
{
}

void CPlayerStatusHUD::SetHP(float current, float max)
{
	currentHP = current;
	maxHP = max;
}

void CPlayerStatusHUD::SetMP(float current, float max)
{
	currentMP = current;
	maxMP = max;
}

void CPlayerStatusHUD::Init()
{
	imgFrame = LOADIMAGE(L"Player_Status", L"Image/Sheet/Player_Status.bmp");
	imgHPBar = LOADIMAGE(L"Player_HP", L"Image/Sheet/Player_HP.bmp");
	imgMPBar = LOADIMAGE(L"Player_MP", L"Image/Sheet/Player_MP.bmp");

	// Set scale to frame size
	if (imgFrame)
	{
		scale = Vec2((float)imgFrame->GetBmpWidth(), (float)imgFrame->GetBmpHeight());
	}
}

void CPlayerStatusHUD::OnEnable()
{
}

void CPlayerStatusHUD::Update()
{
}

void CPlayerStatusHUD::Render()
{
	if (!imgFrame || !imgHPBar || !imgMPBar)
		return;

	// 1. HP bar
	float hpRatio = (maxHP > 0.f) ? (currentHP / maxHP) : 0.f;
	float hpFullWidth = (float)imgHPBar->GetBmpWidth();
	float hpHeight = (float)imgHPBar->GetBmpHeight();
	float hpWidth = hpFullWidth * hpRatio;

	float hpX = renderPos.x + HP_BAR_OFFSET_X;
	float hpY = renderPos.y + HP_BAR_OFFSET_Y;

	RENDER->FrameImage(imgHPBar,
		hpX, hpY, hpX + hpWidth, hpY + hpHeight,
		0, 0, hpWidth, hpHeight,
		false);

	// 2. MP bar
	float mpRatio = (maxMP > 0.f) ? (currentMP / maxMP) : 0.f;
	float mpFullWidth = (float)imgMPBar->GetBmpWidth();
	float mpHeight = (float)imgMPBar->GetBmpHeight();
	float mpWidth = mpFullWidth * mpRatio;

	float mpX = renderPos.x + MP_BAR_OFFSET_X;
	float mpY = renderPos.y + MP_BAR_OFFSET_Y;

	RENDER->FrameImage(imgMPBar,
		mpX, mpY, mpX + mpWidth, mpY + mpHeight,
		0, 0, mpWidth, mpHeight,
		false);

	// 3. portrait, frame
	RENDER->TransparentImage(imgFrame,
		renderPos.x, renderPos.y,
		renderPos.x + scale.x, renderPos.y + scale.y);
}

void CPlayerStatusHUD::OnDisable()
{
}

void CPlayerStatusHUD::Release()
{
}
