#include "pch.h"
#include "Panel.h"

Panel::Panel()
{
	dragStartPos	= Vec2(0, 0);
	isDragging		= false;
	draggable		= true;
}

Panel::~Panel()
{
}

void Panel::Init()
{
}

void Panel::OnEnable()
{
}

void Panel::Update()
{
	if (isDragging)
	{
		Vec2 diff = INPUT->MouseScreenPos() - dragStartPos;
		pos += diff;
		dragStartPos = INPUT->MouseScreenPos();
	}
}

void Panel::Render()
{
	RENDER->Rect(
		renderPos.x,
		renderPos.y,
		renderPos.x + scale.x,
		renderPos.y + scale.y
	);
}

void Panel::OnDisable()
{
}

void Panel::Release()
{
}

void Panel::OnMouseEnter()
{
}

void Panel::OnMouseOver()
{
}

void Panel::OnMouseExit()
{ 
}

void Panel::OnMouseUp()
{
	isDragging = false;
}

void Panel::OnMouseDown()
{
	dragStartPos = INPUT->MouseScreenPos();
	isDragging = draggable;
}

void Panel::OnMouseClicked()
{
}
