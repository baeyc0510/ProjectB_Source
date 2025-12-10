#pragma once

class UIBase;

class UIManager : public SingleTon<UIManager>
{
	friend SingleTon<UIManager>;
private:
	UIManager();
	virtual ~UIManager();

public:
	void Init();
	void Update();
	void Release();

	UIBase* GetFocusedUI();
	void SetFocusedUI(UIBase* ui);

private:
	void MouseEvent(UIBase* ui, UIBase* topChildUI);

	UIBase* GetTopUI();
	UIBase* GetTopChildUI(UIBase* parentUI);

private:
	UIBase* focusedUI;
};

