#include "pch.h"
#include "CInventoryUI.h"
#include "Asset/CImage.h"

CInventoryUI::CInventoryUI()
	: imgBackground(nullptr)
{
}

CInventoryUI::~CInventoryUI()
{
}

void CInventoryUI::Init()
{
	imgBackground = LOADIMAGE(L"Inventory_Base", L"Image/Sheet/inventory_base.bmp");
	SetFullscreen(imgBackground);
}

void CInventoryUI::OnEnable()
{
}

void CInventoryUI::Update()
{
}

void CInventoryUI::Render()
{
	if (!imgBackground)
		return;
	
	RENDER->TransparentImage(imgBackground,
		renderPos.x, renderPos.y,
		renderPos.x + scale.x, renderPos.y + scale.y);
}

void CInventoryUI::OnDisable()
{
}

void CInventoryUI::Release()
{
}
