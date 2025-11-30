#include "pch.h"
#include "CInventoryUI.h"
#include "Asset/CImage.h"
#include "Game/CGame.h"

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

	// 화면 전체 크기로 설정
	pos = Vec2(0.f, 0.f);
	scale = CGame::WINSIZE;
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
