#include "pch.h"
#include "CButton_MainMenu.h"


CButton_MainMenu::CButton_MainMenu(const wstring& buttonName)
{
    name = buttonName;
}

CButton_MainMenu::CButton_MainMenu(const wstring& buttonName, const wstring& inButtonOnImgPath,
    const wstring& inButtonOffImgPath)
{
    name = buttonName;
    buttonOnImgPath = inButtonOnImgPath;
    buttonOffImgPath = inButtonOffImgPath;
}

void CButton_MainMenu::Init()
{
    imgOn = LOADIMAGE(name+L"On", buttonOnImgPath);
    imgOff = LOADIMAGE(name+L"Off", buttonOffImgPath);
    currentImg = imgOff;
    
    InitScale();
}

void CButton_MainMenu::Render()
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

void CButton_MainMenu::OnEnable()
{
}

void CButton_MainMenu::Update()
{
}

void CButton_MainMenu::OnDisable()
{
}

void CButton_MainMenu::Release()
{
}

void CButton_MainMenu::OnMouseClicked()
{
    onButtonPressed.Invoke();
}

void CButton_MainMenu::OnMouseEnter()
{
    onButtonHover.Invoke();
}

void CButton_MainMenu::OnMouseExit()
{
    SetButtonActive(false);
}

void CButton_MainMenu::InitScale()
{
    if (currentImg != nullptr)
    {
        scale = Vec2((float)currentImg->GetBmpWidth(), (float)currentImg->GetBmpHeight()) * buttonScale;
    }
}

void CButton_MainMenu::SetButtonActive(bool bActive)
{
    currentImg = bActive ? imgOn : imgOff;
    InitScale();
}

void CButton_MainMenu::SetButtonScale(float inScale)
{
    buttonScale = inScale;
    InitScale();
}
