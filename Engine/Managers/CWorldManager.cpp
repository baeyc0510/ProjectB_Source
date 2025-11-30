#include "pch.h"
#include "CWorldManager.h"

CWorldManager::CWorldManager()
{
	changeSceneEvent = nullptr;
}

CWorldManager::~CWorldManager()
{
}

void CWorldManager::Init()
{
}

void CWorldManager::Update()
{
	ProgressAddGameObject();
	ProgressDeleteObject();
	ProgressAddComponent();

	ProgressAddUI();
	ProgressAddChildUI();
	ProgressDeleteUI();
	ProgressShowUI();

	ProgressChangeScene();
}

void CWorldManager::Release()
{
}

void CWorldManager::AddGameObject(CScene* scene, CGameObject* obj)
{
	addGameObjectQueue.push(make_pair(scene, obj));
}

void CWorldManager::AddChild(CGameObject* parent, Component<CGameObject>* child)
{
	addChildQueue.push(make_pair(parent, child));
}

void CWorldManager::Delete(CScene* scene, Component<CGameObject>* obj)
{
	deleteObjectQueue.push(make_pair(scene, obj));
}

void CWorldManager::AddUI(CScene* scene, CUI* ui)
{
	addUIQueue.push(make_pair(scene, ui));
}

void CWorldManager::AddChild(CUI* parent, CUI* child)
{
	addChildUIQueue.push(make_pair(parent, child));
}

void CWorldManager::Delete(CScene* scene, CUI* ui)
{
	deleteUIQueue.push(make_pair(scene, ui));
}

void CWorldManager::ShowUI(CUI* ui, bool show)
{
	showUIQueue.push(make_pair(ui, show));
}

void CWorldManager::ChangeScene(int sceneType, float delay)
{
	// 씬 전환 이벤트를 자료구조에 보관
	if (nullptr == changeSceneEvent)
	{
		changeSceneEvent = new pair<int, float>(sceneType, delay);
	}
	else if (changeSceneEvent->second > delay)
	{
		delete changeSceneEvent;
		changeSceneEvent = new pair<int, float>(sceneType, delay);
	}
	else
	{
		// 딜레이가 더욱 큰 씬전환 이벤트는 무시
	}
}

void CWorldManager::ProgressAddGameObject()
{
	while (!addGameObjectQueue.empty())
	{
		CScene* scene = addGameObjectQueue.front().first;
		CGameObject* obj = addGameObjectQueue.front().second;
		addGameObjectQueue.pop();
		scene->AddGameObject(obj);
	}
}

void CWorldManager::ProgressAddComponent()
{
	while (!addChildQueue.empty())
	{
		CGameObject* parent = addChildQueue.front().first;
		Component<CGameObject>* child = addChildQueue.front().second;
		addChildQueue.pop();
		parent->AddChild(child);
	}
}

void CWorldManager::ProgressDeleteObject()
{
	// 삭제 예정 표시된 게임오브젝트를 삭제 진행
	CScene* curScene = SINGLE(CSceneManager)->GetCurScene();
	list<CGameObject*>& objList = curScene->objList;

	objList.remove_if([&](CGameObject* obj) {
		if (obj->IsReservedDelete())
		{
			if (curScene->active) obj->ComponentOnDisable();
			obj->ComponentRelease();
			obj->SetScene(nullptr);
			delete obj;
			return true;
		}
		else
		{
			obj->DeleteReservedChild();
			return false;
		}
		});

	// 삭제 예정인 게임오브젝트에 삭제예정 표시를 진행
	while (!deleteObjectQueue.empty())
	{
		Component<CGameObject>* component = deleteObjectQueue.front().second;
		deleteObjectQueue.pop();
		component->SetReservedDelete();
	}
}

void CWorldManager::ProgressAddUI()
{
	while (!addUIQueue.empty())
	{
		CScene* scene = addUIQueue.front().first;
		CUI* ui = addUIQueue.front().second;
		addUIQueue.pop();
		scene->AddUI(ui);
	}
}

void CWorldManager::ProgressAddChildUI()
{
	while (!addChildUIQueue.empty())
	{
		CUI* parent = addChildUIQueue.front().first;
		CUI* child = addChildUIQueue.front().second;
		addChildUIQueue.pop();
		parent->AddChild(child);
	}
}

void CWorldManager::ProgressDeleteUI()
{
	// 삭제 예정 표시된 UI를 삭제 진행
	CScene* curScene = SINGLE(CSceneManager)->GetCurScene();
	list<CUI*>& uiList = curScene->uiList;

	uiList.remove_if([&](CUI* ui) {
		if (ui->IsReservedDelete())
		{
			if (curScene->active) ui->ComponentOnDisable();
			ui->ComponentRelease();
			ui->SetScene(nullptr);
			delete ui;
			return true;
		}
		else
		{
			ui->DeleteReservedChild();
			return false;
		}
		});

	// 삭제 예정인 게임오브젝트에 삭제예정 표시를 진행
	while (!deleteUIQueue.empty())
	{
		CUI* child = deleteUIQueue.front().second;
		deleteUIQueue.pop();
		child->SetReservedDelete();
	}
}

void CWorldManager::ProgressShowUI()
{
	while (!showUIQueue.empty())
	{
		CUI* ui = showUIQueue.front().first;
		bool show = showUIQueue.front().second;
		showUIQueue.pop();
		ui->SetShow(show);
	}
}

void CWorldManager::ProgressChangeScene()
{
	if (nullptr == changeSceneEvent)
		return;

	// 지연실행 이벤트가 잔여시간이 모두 소진되었을 경우 이벤트 진행
	changeSceneEvent->second -= DT;
	if (changeSceneEvent->second <= 0)
	{
		int scene = changeSceneEvent->first;
		delete changeSceneEvent;
		changeSceneEvent = nullptr;
		SINGLE(CSceneManager)->ChangeScene(scene);
	}
}
