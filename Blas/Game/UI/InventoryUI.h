#pragma once
#include "OverlayUI.h"

class ImageResource;

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
	ImageResource* imgBackground;
};
