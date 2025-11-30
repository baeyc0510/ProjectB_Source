#pragma once
#include "UI/CUI.h"

class CImage;

class CInventoryUI : public CUI
{
public:
	CInventoryUI();
	~CInventoryUI() override;

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
