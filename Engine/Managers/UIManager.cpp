#include "pch.h"
#include "UIManager.h"

UIManager::UIManager()
{
	focusedUI = nullptr;
}

UIManager::~UIManager()
{
}

void UIManager::Init()
{
}

void UIManager::Update()
{
	UIBase* topUI = GetTopUI();
	UIBase* topChildUI = GetTopChildUI(topUI);

	if (INPUT->ButtonDown(VK_LBUTTON, true))
		SetFocusedUI(topUI);

	Scene* pCurScene = SINGLE(SceneManager)->GetCurScene();
	const list<UIBase*>& uiList = pCurScene->uiList;
	for (UIBase* ui : uiList)
	{
		MouseEvent(ui, topChildUI);
	}
}

void UIManager::Release()
{
}

UIBase* UIManager::GetFocusedUI()
{
	return focusedUI;
}

void UIManager::SetFocusedUI(UIBase* ui)
{
	// 이미 포커싱된 UI일 경우 진행하지 않음
	if (focusedUI == ui)
		return;

	// 포커싱을 nullptr로 지정할 경우 포커싱 UI 를 nullptr로 설정
	if (nullptr == ui)
	{
		focusedUI = nullptr;
		return;
	}

	focusedUI = ui;

	// 포커싱된 UI를 최상단에 배치하도록 자료구조의 가장 뒤에 배치
	Scene* curScene = SINGLE(SceneManager)->GetCurScene();
	list<UIBase*>& listUI = curScene->uiList;

	listUI.remove(focusedUI);
	listUI.push_back(focusedUI);
}

void UIManager::MouseEvent(UIBase* ui, UIBase* topChildUI)
{
	if (ui->IsMouseOn() && ui->IsShow() && ui == topChildUI)
	{
		if (!ui->prevMouseOn)
		{
			ui->OnMouseEnter();
		}
		ui->prevMouseOn = true;
		ui->OnMouseOver();
		if (INPUT->ButtonDown(VK_LBUTTON, true))
		{
			ui->OnMouseDown();
			ui->prevDown = true;
		}
		else if (INPUT->ButtonUp(VK_LBUTTON, true))
		{
			ui->OnMouseUp();

			if (ui->prevDown)
			{
				ui->OnMouseClicked();
			}
			ui->prevDown = false;
		}
	}
	else
	{
		if (ui->prevMouseOn)
		{
			ui->OnMouseExit();		
		}
		ui->prevMouseOn = false;
		if (INPUT->ButtonUp(VK_LBUTTON, true))
		{
			ui->prevDown = false;
		}
	}

	for (Component<UIBase>* childUI : ui->childList)
	{
		MouseEvent((UIBase*)childUI, topChildUI);
	}
}

UIBase* UIManager::GetTopUI()
{
	Scene* curScene = SINGLE(SceneManager)->GetCurScene();
	const list<UIBase*>& uiList = curScene->uiList;

	// 가장 위에 있는 UI는 씬의 게임오브젝트 자료구조에서 차례대로 순회하며 Render 한다는 것을 이용하여
	// 가장 마지막에 있는 게임오브젝트가 제일 위에 있는 게임오브젝트
	// 역방향 반복자 : 가장 뒤의 요소부터 역순으로 순회
	for (auto iter = uiList.rbegin(); iter != uiList.rend(); iter++)
	{
		// 역순으로 순회 중 가장먼저 마우스가 올려져 있는 UI가 마우스 클릭의 대상
		UIBase* pUI = static_cast<UIBase*>(*iter);
		if (pUI->IsMouseOn() && pUI->IsShow())
		{
			return pUI;
		}
	}

	return nullptr;
}

UIBase* UIManager::GetTopChildUI(UIBase* parentUI)
{
	if (nullptr == parentUI)
		return nullptr;

	queue<UIBase*> queueUI;
	UIBase* topChildUI = nullptr;

	// BFS 을 통해서 UI중 마우스가 올라간 가장 자식 UI 확인
	queueUI.push(parentUI);
	while (!queueUI.empty())
	{
		UIBase* ui = queueUI.front();
		queueUI.pop();

		if (ui->IsMouseOn())
		{
			topChildUI = ui;
		}

		for (Component<UIBase>* childUI : ui->childList)
		{
			queueUI.push((UIBase*)childUI);
		}
	}

	return topChildUI;
}
