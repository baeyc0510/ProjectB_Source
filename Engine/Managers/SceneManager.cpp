#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	curScene = nullptr;
}

SceneManager::~SceneManager()
{
}

void SceneManager::Init()
{
}

void SceneManager::Update()
{
	curScene->SceneUpdate();
}

void SceneManager::Render()
{
	curScene->SceneRender();
}

void SceneManager::Release()
{
	// 게임씬 자료구조를 순회하며 동적할당된 씬을 제거
	for (pair<int, Scene*> scene : mapScene)
	{
		scene.second->SceneRelease();
		delete scene.second;
	}
	mapScene.clear();
}

Scene* SceneManager::FindScene(int key)
{
	auto iter = mapScene.find(key);
	if (iter == mapScene.end())
		return nullptr;
	else
		return iter->second;
}

void SceneManager::AddScene(int key, Scene* scene)
{
	// 같은 키값의 씬이 이미 있을 경우
	Scene* findScene = FindScene(key);
	assert(nullptr != scene && "Scene already exist");

	scene->SetSceneId(key);
	scene->SceneInit();
	mapScene.insert(make_pair(key, scene));
}

void SceneManager::RemoveScene(int key)
{
	Scene* findScene = FindScene(key);
	if (nullptr == findScene) return;

	mapScene.erase(key);
	findScene->SceneRelease();
	delete findScene;
}

void SceneManager::ChangeScene(int key)
{
	Scene* scene = FindScene(key);
	assert(nullptr != scene && "Scene do not exist");

	// Persistent 오브젝트 추출 (씬 전환 전에)
	list<GameObject*> persistentObjs;
	for (GameObject* obj : curScene->objList)
	{
		if (obj->IsPersistent())
		{
			persistentObjs.push_back(obj);
		}
	}

	// 이전 씬에서 persistent 오브젝트 제거 (삭제 없이)
	for (GameObject* obj : persistentObjs)
	{
		curScene->RemoveGameObject(obj);
	}

	// 이전 씬을 Exit
	curScene->SceneExit();
	curScene = scene;

	// 새 씬에 persistent 오브젝트 추가 (SceneEnter 전에, Init 없이)
	for (GameObject* obj : persistentObjs)
	{
		curScene->AddGameObjectWithoutInit(obj);
	}

	// 다음 씬을 Enter
	curScene->SceneEnter();
}

void SceneManager::SetStartScene(int key)
{
	Scene* scene = FindScene(key);
	assert(nullptr != scene && "Scene do not exist");

	curScene = scene;
	curScene->SceneEnter();
}

Scene* SceneManager::GetCurScene()
{
	return curScene;
}

int SceneManager::GetCurSceneKey()
{
	if (!curScene)
		return -1;
	
	return curScene->GetSceneId();
}

void SceneManager::ResetAllGameScenes()
{
	for (auto& pair : mapScene)
	{
		Scene* scene = pair.second;
		scene->ResetScene();
	}
}

