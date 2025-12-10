#pragma once
#include "OverlayUI.h"

class CImage;

class InventoryUI : public OverlayUI
{
public:
	InventoryUI();
	~InventoryUI() override;

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
