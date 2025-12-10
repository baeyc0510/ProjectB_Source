#include "pch.h"
#include "UIBase.h"

UIBase::UIBase()
{
	name		= TEXT("");
	pos			= Vec2(0, 0);
	worldPos	= Vec2(0, 0);
	renderPos	= Vec2(0, 0);
	scale		= Vec2(0, 0);
	screenFixed	= true;
	show		= true;
}

UIBase::~UIBase()
{
}

bool UIBase::IsMouseOn()
{
	if (renderPos.x <= INPUT->MouseScreenPos().x && INPUT->MouseScreenPos().x <= renderPos.x + scale.x &&
		renderPos.y <= INPUT->MouseScreenPos().y && INPUT->MouseScreenPos().y <= renderPos.y + scale.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void UIBase::ComponentInit()
{
	Init();
	Composite::ComponentInit();
}

void UIBase::ComponentOnEnable()
{
	OnEnable();
	Composite::ComponentOnEnable();
}

void UIBase::ComponentUpdate()
{
	if (IsReservedDelete())
		return;

	// 부모 게임오브젝트가 있는 경우, 위치는 부모를 기준으로한 상대위치
	if (GetOwner() != nullptr)
		worldPos = GetOwner()->GetWorldPos() + pos;
	// 부모 게임오브젝트가 없는 경우, 위치는 월드를 기준으로한 절대위치
	else
		worldPos = pos;

	if (IsScreenFixed())
		renderPos = worldPos;
	else
		renderPos = SINGLE(CameraManager)->WorldToScreenPoint(worldPos);

	Update();
	Composite::ComponentUpdate();
}

void UIBase::ComponentRender()
{
	if (IsReservedDelete() || !IsShow())
		return;

	Render();
	Composite::ComponentRender();
}

void UIBase::ComponentOnDisable()
{
	Composite::ComponentOnDisable();
	OnDisable();
}

void UIBase::ComponentRelease()
{
	Composite::ComponentRelease();
	Release();
}
