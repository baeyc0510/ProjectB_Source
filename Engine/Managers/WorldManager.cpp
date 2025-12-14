#include "pch.h"
#include "WorldManager.h"

WorldManager::WorldManager()
{
	changeSceneEvent = nullptr;
}

WorldManager::~WorldManager()
{
}

void WorldManager::Init()
{
}

void WorldManager::Update()
{
	ProgressAddGameObject();
	ProgressDeleteObject();
	ProgressAddComponent();

	ProgressAddUI();
	ProgressAddChildUI();
	ProgressDeleteUI();
	ProgressShowUI();

	ProgressChangeScene();
	ProgressResetScene();
}

void WorldManager::Release()
{
}

void WorldManager::AddGameObject(Scene* scene, GameObject* obj)
{
	addGameObjectQueue.push(make_pair(scene, obj));
}

void WorldManager::AddChild(GameObject* parent, Component<GameObject>* child)
{
	addChildQueue.push(make_pair(parent, child));
}

void WorldManager::Delete(Scene* scene, Component<GameObject>* obj)
{
	deleteObjectQueue.push(make_pair(scene, obj));
}

void WorldManager::AddUI(Scene* scene, UIBase* ui)
{
	addUIQueue.push(make_pair(scene, ui));
}

void WorldManager::AddChild(UIBase* parent, UIBase* child)
{
	addChildUIQueue.push(make_pair(parent, child));
}

void WorldManager::Delete(Scene* scene, UIBase* ui)
{
	deleteUIQueue.push(make_pair(scene, ui));
}

void WorldManager::ShowUI(UIBase* ui, bool show)
{
	showUIQueue.push(make_pair(ui, show));
}

void WorldManager::ChangeScene(int sceneType, float delay)
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

void WorldManager::ProgressAddGameObject()
{
	while (!addGameObjectQueue.empty())
	{
		Scene* scene = addGameObjectQueue.front().first;
		GameObject* obj = addGameObjectQueue.front().second;
		addGameObjectQueue.pop();
		scene->AddGameObject(obj);
	}
}

void WorldManager::ProgressAddComponent()
{
	while (!addChildQueue.empty())
	{
		GameObject* parent = addChildQueue.front().first;
		Component<GameObject>* child = addChildQueue.front().second;
		addChildQueue.pop();
		parent->AddChild(child);
	}
}

void WorldManager::ProgressDeleteObject()
{
	// 삭제 예정 표시된 게임오브젝트를 삭제 진행
	Scene* curScene = SINGLE(SceneManager)->GetCurScene();
	list<GameObject*>& objList = curScene->objList;

	objList.remove_if([&](GameObject* obj) {
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
		Component<GameObject>* component = deleteObjectQueue.front().second;
		deleteObjectQueue.pop();
		component->SetReservedDelete();
	}
}

void WorldManager::ProgressAddUI()
{
	while (!addUIQueue.empty())
	{
		Scene* scene = addUIQueue.front().first;
		UIBase* ui = addUIQueue.front().second;
		addUIQueue.pop();
		scene->AddUI(ui);
	}
}

void WorldManager::ProgressAddChildUI()
{
	while (!addChildUIQueue.empty())
	{
		UIBase* parent = addChildUIQueue.front().first;
		UIBase* child = addChildUIQueue.front().second;
		addChildUIQueue.pop();
		parent->AddChild(child);
	}
}

void WorldManager::ProgressDeleteUI()
{
	// 삭제 예정 표시된 UI를 삭제 진행
	Scene* curScene = SINGLE(SceneManager)->GetCurScene();
	list<UIBase*>& uiList = curScene->uiList;

	uiList.remove_if([&](UIBase* ui) {
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
		UIBase* child = deleteUIQueue.front().second;
		deleteUIQueue.pop();
		child->SetReservedDelete();
	}
}

void WorldManager::ProgressShowUI()
{
	while (!showUIQueue.empty())
	{
		UIBase* ui = showUIQueue.front().first;
		bool show = showUIQueue.front().second;
		showUIQueue.pop();
		ui->SetShow(show);
	}
}

void WorldManager::ProgressChangeScene()
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
		SINGLE(SceneManager)->ChangeScene(scene);
	}
}

void WorldManager::ResetScene(Scene* scene)
{
	resetSceneQueue.push(scene);
}

void WorldManager::ResetAllScenes()
{
	const map<int, Scene*>& allScenes = SINGLE(SceneManager)->GetAllScenes();
	for (const auto& pair : allScenes)
	{
		resetSceneQueue.push(pair.second);
	}
}

void WorldManager::ProgressResetScene()
{
	while (!resetSceneQueue.empty())
	{
		Scene* scene = resetSceneQueue.front();
		resetSceneQueue.pop();
		scene->ResetScene();
	}
}
