#include "pch.h"
#include "Button.h"

Button::Button()
{
	callback	= nullptr;
	param1		= 0;
	param2		= 0;
}

Button::~Button()
{
}

void Button::SetClickCallback(CallbackFunc callback, DWORD_PTR param1, DWORD_PTR param2)
{
	this->callback	= callback;
	this->param1	= param1;
	this->param2	= param2;
}

void Button::Init()
{
}

void Button::OnEnable()
{
}

void Button::Update()
{
}

void Button::Render()
{
	RENDER->Rect(
		renderPos.x,
		renderPos.y,
		renderPos.x + scale.x,
		renderPos.y + scale.y
	);
}

void Button::OnDisable()
{
}

void Button::Release()
{
}

void Button::OnMouseEnter()
{
}

void Button::OnMouseOver()
{
}

void Button::OnMouseExit()
{
}

void Button::OnMouseUp()
{
}

void Button::OnMouseDown()
{
}

void Button::OnMouseClicked()
{
	if (nullptr != callback)
		callback(param1, param2);
}
