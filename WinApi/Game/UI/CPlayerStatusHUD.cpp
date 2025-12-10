#include "pch.h"
#include "CPlayerStatusHUD.h"
#include "Asset/CImage.h"

CPlayerStatusHUD::CPlayerStatusHUD()
	: imgPlayerFrame(nullptr)
	, imgPlayerHPBar(nullptr)
	, imgPlayerMPBar(nullptr)
	, currentHP(100.f)
	, maxHP(100.f)
	, currentMP(100.f)
	, maxMP(100.f)
	, hudScale(1.2f)
{
}

CPlayerStatusHUD::~CPlayerStatusHUD()
{
	
}

void CPlayerStatusHUD::SetVisibility(bool visibility)
{
	SetShow(visibility);
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

void CPlayerStatusHUD::SetFlask(int current, int max)
{
	currentFlask = current;
	maxFlask = max;
}

void CPlayerStatusHUD::Init()
{
	imgPlayerFrame = LOADIMAGE(L"Player_Status", L"Image/Sheet/Player_Status.bmp");
	imgPlayerHPBar = LOADIMAGE(L"Player_HP", L"Image/Sheet/Player_HP.bmp");
	imgPlayerMPBar = LOADIMAGE(L"Player_MP", L"Image/Sheet/Player_MP.bmp");
	imgPlayerFlaskFull = LOADIMAGE(L"Flask_Full", L"Image/Sheet/Full_Flask.bmp");
	imgPlayerFlaskEmpty = LOADIMAGE(L"Flask_Empty", L"Image/Sheet/Empty_Flask.bmp");

	// Set scale to frame size
	if (imgPlayerFrame)
	{
		scale = Vec2((float)imgPlayerFrame->GetBmpWidth(), (float)imgPlayerFrame->GetBmpHeight());
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
	if (!imgPlayerFrame || !imgPlayerHPBar || !imgPlayerMPBar)
		return;

	// UI는 별도 레이어에서 윈도우 해상도(1280x720)로 직접 렌더링됨
	float s = hudScale;
	float baseX = renderPos.x;
	float baseY = renderPos.y;

	// 1. HP bar
	float hpRatio = (maxHP > 0.f) ? (currentHP / maxHP) : 0.f;
	float hpSrcWidth = (float)imgPlayerHPBar->GetBmpWidth() * hpRatio;
	float hpSrcHeight = (float)imgPlayerHPBar->GetBmpHeight();
	float hpDstWidth = hpSrcWidth * s;
	float hpDstHeight = hpSrcHeight * s;

	float hpX = baseX + HP_BAR_OFFSET_X * s;
	float hpY = baseY + HP_BAR_OFFSET_Y * s;

	RENDER->FrameImage(imgPlayerHPBar,
		hpX, hpY, hpX + hpDstWidth, hpY + hpDstHeight,
		0, 0, hpSrcWidth, hpSrcHeight,
		false);

	// 2. MP bar
	float mpRatio = (maxMP > 0.f) ? (currentMP / maxMP) : 0.f;
	float mpSrcWidth = (float)imgPlayerMPBar->GetBmpWidth() * mpRatio;
	float mpSrcHeight = (float)imgPlayerMPBar->GetBmpHeight();
	float mpDstWidth = mpSrcWidth * s;
	float mpDstHeight = mpSrcHeight * s;

	float mpX = baseX + MP_BAR_OFFSET_X * s;
	float mpY = baseY + MP_BAR_OFFSET_Y * s;

	RENDER->FrameImage(imgPlayerMPBar,
		mpX, mpY, mpX + mpDstWidth, mpY + mpDstHeight,
		0, 0, mpSrcWidth, mpSrcHeight,
		false);

	// 3. Flasks
	float flaskX = FLASK_OFFSET_X * s;
	float flaskY = FLASK_OFFSET_Y * s;

	for (int i = 0; i < maxFlask; i++)
	{
		CImage* imgFlask;
		float flaskSrcWidth;
		float flaskSrcHeight;

		// full flask
		if (i < currentFlask)
		{
			imgFlask = imgPlayerFlaskFull;
			flaskSrcWidth = (float)imgPlayerFlaskFull->GetBmpWidth();
			flaskSrcHeight = (float)imgPlayerFlaskFull->GetBmpHeight();
		}
		// empty flask
		else
		{
			imgFlask = imgPlayerFlaskEmpty;
			flaskSrcWidth = (float)imgPlayerFlaskEmpty->GetBmpWidth();
			flaskSrcHeight = (float)imgPlayerFlaskEmpty->GetBmpHeight();
		}

		float flaskDstWidth = flaskSrcWidth * s;
		float flaskDstHeight = flaskSrcHeight * s;

		RENDER->FrameImage(imgFlask,
			flaskX, flaskY, flaskX + flaskDstWidth, flaskY + flaskDstHeight,
			0, 0, flaskSrcWidth, flaskSrcHeight,
			false);

		flaskX += flaskDstWidth + FLASK_SPACE_X * s;
	}

	// 4. portrait, frame
	float frameW = scale.x * s;
	float frameH = scale.y * s;
	RENDER->TransparentImage(imgPlayerFrame,
		baseX, baseY,
		baseX + frameW, baseY + frameH);
}

void CPlayerStatusHUD::OnDisable()
{
}

void CPlayerStatusHUD::Release()
{
}
