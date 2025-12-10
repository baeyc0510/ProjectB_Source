#pragma once
#include "UI/UIBase.h"

class ImageResource;

class PlayerStatusHUD : public UIBase
{
public:
	PlayerStatusHUD();
	~PlayerStatusHUD() override;

public:
	void SetVisibility(bool visibility);
	void SetHP(float current, float max);
	void SetMP(float current, float max);
	void SetFlask(int current, int max);
	void SetHUDScale(float scale) { hudScale = scale; }
	
private:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

private:
	// 오프셋 상수들
	static constexpr float HP_BAR_OFFSET_X = 100.f;
	static constexpr float HP_BAR_OFFSET_Y = 22.f;
	static constexpr float MP_BAR_OFFSET_X = 158.f;
	static constexpr float MP_BAR_OFFSET_Y = 46.f;
	static constexpr float FLASK_OFFSET_X =  145.0f;
	static constexpr float FLASK_OFFSET_Y =  90.0f;
	static constexpr float FLASK_SPACE_X =  10.0f;
	
	// Images
	ImageResource* imgPlayerFrame;		// Player_Status.bmp (foreground frame)
	ImageResource* imgPlayerHPBar;		// Player_HP.bmp (background)
	ImageResource* imgPlayerMPBar;		// Player_MP.bmp (background)
	ImageResource* imgPlayerFlaskEmpty;
	ImageResource* imgPlayerFlaskFull;

	// HP/MP values
	float currentHP;
	float maxHP;
	float currentMP;
	float maxMP;
	
	// flasks
	int currentFlask;
	int maxFlask;

	// HUD scale
	float hudScale;
};
