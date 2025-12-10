#include "pch.h"
#include "InventoryUI.h"
#include "Asset/CImage.h"

InventoryUI::InventoryUI()
	: imgBackground(nullptr)
{
}

InventoryUI::~InventoryUI()
{
}

void InventoryUI::Init()
{
	imgBackground = LOADIMAGE(L"Inventory_Base", L"Image/Sheet/inventory_base.bmp");
	SetFullscreen(imgBackground);
}

void InventoryUI::OnEnable()
{
}

void InventoryUI::Update()
{
}

void InventoryUI::Render()
{
	if (!imgBackground)
		return;
	
	RENDER->TransparentImage(imgBackground,
		renderPos.x, renderPos.y,
		renderPos.x + scale.x, renderPos.y + scale.y);
}

void InventoryUI::OnDisable()
{
}

void InventoryUI::Release()
{
}
