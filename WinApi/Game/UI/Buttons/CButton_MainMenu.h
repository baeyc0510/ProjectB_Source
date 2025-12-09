#pragma once

class CButton_MainMenu : public CUI
{
public:
    CButton_MainMenu(const wstring& buttonName);
    CButton_MainMenu(const wstring& buttonName, const wstring& inButtonOnImgPath, const wstring& inButtonOffImgPath);
    ~CButton_MainMenu() override = default;
    
    void Init() override;
    void Render() override;
    void OnEnable()	override;
    void Update() override;
    void OnDisable() override;
    void Release() override;
    
    void OnMouseClicked() override;
    void OnMouseEnter() override;
    void OnMouseExit() override;
    void InitScale();

    void SetButtonActive(bool bActive);
    void SetButtonScale(float inScale);

public:
    Delegate<> onButtonPressed;
    Delegate<> onButtonHover;
    
private:
    wstring buttonOnImgPath;
    wstring buttonOffImgPath;
    
    CImage* imgOn = nullptr;
    CImage* imgOff = nullptr;
    CImage* currentImg = nullptr;
    
    float buttonScale = 1.0f;
};
