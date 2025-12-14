#include "pch.h"
#include "Scene.h"

Scene::Scene()
{
	active = false;
}

Scene::~Scene()
{
}

void Scene::AddGameObject(GameObject* obj)
{
	objList.push_back(obj);
	obj->SetScene(this);
	obj->ComponentInit();
	if (active) obj->ComponentOnEnable();
}

void Scene::AddGameObjectWithoutInit(GameObject* obj)
{
	// Init 없이 추가 (이미 초기화된 persistent 오브젝트 이동용)
	objList.push_back(obj);
	obj->SetScene(this);
	if (active) obj->ComponentOnEnable();
}

void Scene::RemoveGameObject(GameObject* obj)
{
	// 삭제 없이 리스트에서만 제거 (persistent 오브젝트 이동용)
	if (active) obj->ComponentOnDisable();
	obj->SetScene(nullptr);
	objList.remove(obj);
}

void Scene::DeleteGameObject(GameObject* obj)
{
	if (active) obj->ComponentOnDisable();
	obj->ComponentRelease();
	obj->SetScene(nullptr);
	objList.remove(obj);
	delete obj;
}

void Scene::DeleteAllObject()
{
	list<GameObject*> toDelete;
	for (GameObject* obj : objList)
	{
		if (!obj->IsPersistent())
		{
			toDelete.push_back(obj);
		}
	}
	for (GameObject* obj : toDelete)
	{
		if (active) obj->ComponentOnDisable();
		obj->ComponentRelease();
		objList.remove(obj);
		delete obj;
	}
}

void Scene::DeletePersistentObjects()
{
	list<GameObject*> toDelete;
	for (GameObject* obj : objList)
	{
		if (obj->IsPersistent())
		{
			toDelete.push_back(obj);
		}
	}

	for (GameObject* obj : toDelete)
	{
		DeleteGameObject(obj);
	}
}

void Scene::AddRenderer(IRender* renderer)
{
	renderPQueue.push(make_pair(renderer->zOrder, renderer));
}

void Scene::AddUI(UIBase* ui)
{
	uiList.push_back(ui);
	ui->SetScene(this);
	ui->ComponentInit();
	if (active) ui->ComponentOnEnable();
}

void Scene::DeleteUI(UIBase* ui)
{
	if (active) ui->ComponentOnDisable();
	ui->ComponentRelease();
	ui->SetScene(nullptr);
	uiList.remove(ui);
	delete ui;
}

void Scene::DeleteAllUI()
{
	list<UIBase*> toDelete = uiList;
	for (UIBase* ui : toDelete)
	{
		if (active) ui->ComponentOnDisable();
		ui->ComponentRelease();
		ui->SetScene(nullptr);
		delete ui;
	}
	uiList.clear();
}

void Scene::ResetScene()
{
	Reset();
}

void Scene::SceneInit()
{
	Init();
}

void Scene::SceneEnter()
{
	// 이전 씬의 카메라 상황을 초기화하고 카메라의 위치를 초기 위치로
	// 씬의 카메라 시작위치가 초기 위치가 아닌경우 씬의 Enter에서 위치를 설정하여 목표위치가 덮어씀
	CAMERA->SetTargetObj(nullptr);
	CAMERA->SetTargetPos(SINGLE(EngineInstance)->GetWinSize() * 0.5f);
	CAMERA->FadeIn(0.5f);
	Enter();

	active = true;
	for (GameObject* obj : objList)
	{
		obj->ComponentOnEnable();
	}
	for (UIBase* ui : uiList)
	{
		ui->ComponentOnEnable();
	}
}

void Scene::SceneUpdate()
{
	Update();

	for (GameObject* obj : objList)
	{
		obj->ComponentUpdate();
	}
	for (UIBase* ui : uiList)
	{
		ui->ComponentUpdate();
	}

	// LateUpdate
	for (GameObject* obj : objList)
	{
		obj->ComponentLateUpdate();
	}
	for (UIBase* ui : uiList)
	{
		ui->ComponentLateUpdate();
	}
}

void Scene::SceneRender()
{
	// 1. 배경 레이어 렌더링
	RenderBackground();

	// 2. 게임 오브젝트 렌더링
	for (GameObject* obj : objList)
	{
		obj->ComponentRender();
	}

	while (!renderPQueue.empty())
	{
		pair<float, IRender*> top = renderPQueue.top();
		renderPQueue.pop();
		top.second->Render();
	}

	// 3. 전경 레이어 렌더링
	RenderForeground();

	// 4. UI 렌더링
	RENDER->BeginUI();
	for (UIBase* ui : uiList)
	{
		ui->ComponentRender();
	}
	RENDER->EndUI();

	Render();
}

void Scene::SceneExit()
{
	for (GameObject* obj : objList)
	{
		obj->ComponentOnDisable();
	}
	for (UIBase* ui : uiList)
	{
		ui->ComponentOnDisable();
	}
	active = false;
	Exit();
}

void Scene::SceneRelease()
{
	for (GameObject* obj : objList)
	{
		obj->ComponentRelease();
		delete obj;
	}
	objList.clear();
	for (UIBase* ui : uiList)
	{
		ui->ComponentRelease();
		delete ui;
	}
	uiList.clear();

	Release();
}
