#include "pch.h"
#include "Button_MainMenu.h"


Button_MainMenu::Button_MainMenu(const wstring& buttonName)
{
    name = buttonName;
}

Button_MainMenu::Button_MainMenu(const wstring& buttonName, const wstring& inButtonOnImgPath,
    const wstring& inButtonOffImgPath)
{
    name = buttonName;
    buttonOnImgPath = inButtonOnImgPath;
    buttonOffImgPath = inButtonOffImgPath;
}

void Button_MainMenu::Init()
{
    imgOn = LOADIMAGE(name+L"On", buttonOnImgPath);
    imgOff = LOADIMAGE(name+L"Off", buttonOffImgPath);
    currentImg = imgOff;
    
    InitScale();
}

void Button_MainMenu::Render()
{
    if (!currentImg)
        return;
    
    float baseX = renderPos.x;
    float baseY = renderPos.y;
    
    float frameW = scale.x;
    float frameH = scale.y;
    
    RENDER->TransparentImage(currentImg,
        baseX, baseY,
        baseX + frameW, baseY + frameH);
}

void Button_MainMenu::OnEnable()
{
}

void Button_MainMenu::Update()
{
}

void Button_MainMenu::OnDisable()
{
}

void Button_MainMenu::Release()
{
}

void Button_MainMenu::OnMouseClicked()
{
    onButtonPressed.Invoke();
}

void Button_MainMenu::OnMouseEnter()
{
    onButtonHover.Invoke();
}

void Button_MainMenu::OnMouseExit()
{
    SetButtonActive(false);
}

void Button_MainMenu::InitScale()
{
    if (currentImg != nullptr)
    {
        scale = Vec2((float)currentImg->GetBmpWidth(), (float)currentImg->GetBmpHeight()) * buttonScale;
    }
}

void Button_MainMenu::SetButtonActive(bool bActive)
{
    currentImg = bActive ? imgOn : imgOff;
    InitScale();
}

void Button_MainMenu::SetButtonScale(float inScale)
{
    buttonScale = inScale;
    InitScale();
}
