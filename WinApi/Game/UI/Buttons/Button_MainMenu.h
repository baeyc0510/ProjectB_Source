#pragma once

class Button_MainMenu : public UIBase
{
public:
    Button_MainMenu(const wstring& buttonName);
    Button_MainMenu(const wstring& buttonName, const wstring& inButtonOnImgPath, const wstring& inButtonOffImgPath);
    ~Button_MainMenu() override = default;
    
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
    
    ImageResource* imgOn = nullptr;
    ImageResource* imgOff = nullptr;
    ImageResource* currentImg = nullptr;
    
    float buttonScale = 1.0f;
};
