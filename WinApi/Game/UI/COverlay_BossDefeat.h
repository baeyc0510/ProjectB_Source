#pragma once
#include "COverlayUI.h"

class COverlay_BossDefeat : public COverlayUI
{
public:
private:
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;

private:
    CImage* imgBackground;
};
