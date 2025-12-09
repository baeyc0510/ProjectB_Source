#pragma once

class CBossHUD : public CUI
{
public:
    CBossHUD();
    ~CBossHUD();

public:
    void SetVisibility(bool visibility);
    void SetHP(float current, float max);
    void SetHUDScale(float scale) { hudScale = scale; }
    void SetBossNameImage(CImage* image);
    
private:
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;

private:
    // 오프셋 상수들
    static constexpr float HP_BAR_OFFSET_X = 29.f;
    static constexpr float HP_BAR_OFFSET_Y = 13.f;
    static constexpr float NAME_OFFSET_Y = -50.f;  // 프레임 상단 위
	
    // Images
    CImage* imgFrame;		// foreground frame
    CImage* imgHPBar;		// background
    CImage* imgName;

    // HP values
    float currentHP;
    float maxHP;

    // HUD scale
    float hudScale;

    // Boss name
    float nameWidth;
    float nameHeight;
};
