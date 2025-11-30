#pragma once
#include "UI/CUI.h"

class CImage;

// 오프셋 상수들
static constexpr float HP_BAR_OFFSET_X = 100.f;
static constexpr float HP_BAR_OFFSET_Y = 22.f;
static constexpr float MP_BAR_OFFSET_X = 158.f;
static constexpr float MP_BAR_OFFSET_Y = 46.f;

class CPlayerStatusHUD : public CUI
{
public:
	CPlayerStatusHUD();
	~CPlayerStatusHUD() override;

public:
	void SetHP(float current, float max);
	void SetMP(float current, float max);

private:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

private:
	// Images
	CImage* imgFrame;		// Player_Status.bmp (foreground frame)
	CImage* imgHPBar;		// Player_HP.bmp (background)
	CImage* imgMPBar;		// Player_MP.bmp (background)

	// HP/MP values
	float currentHP;
	float maxHP;
	float currentMP;
	float maxMP;
};
