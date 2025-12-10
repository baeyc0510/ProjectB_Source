#pragma once

class Scene;
class GameObject;
class UIBase;

class WorldManager : public SingleTon<WorldManager>
{
	friend SingleTon<WorldManager>;
private:
	WorldManager();
	virtual ~WorldManager();

public:
	void Init();
	void Update();
	void Release();

	void AddGameObject(Scene* scene, GameObject* obj);							// 게임오브젝트 추가 이벤트 추가
	void AddChild(GameObject* parent, Component<GameObject>* child);				// 컴포넌트 추가 이벤트 추가
	void Delete(Scene* scene, Component<GameObject>* obj);						// 오브젝트 삭제 이벤트 추가

	void AddUI(Scene* scene, UIBase* ui);												// UI 추가 이벤트 추가
	void AddChild(UIBase* parent, UIBase* child);											// 자식UI 추가 이벤트 추가
	void Delete(Scene* scene, UIBase* ui);											// UI 삭제 이벤트 추가
	void ShowUI(UIBase* ui, bool show);												// UI 표시 변경 이벤트 추가

	void ChangeScene(int sceneType, float delay = 0);								// 씬 전환 이벤트 추가

private:
	void ProgressAddGameObject();													// 게임오브젝트 추가 이벤트 진행
	void ProgressAddComponent();													// 컴포넌트 추가 이벤트 진행
	void ProgressDeleteObject();													// 오브젝트 삭제 이벤트 진행

	void ProgressAddUI();															// UI 추가 이벤트 진행
	void ProgressAddChildUI();														// 자식UI 추가 이벤트 진행
	void ProgressDeleteUI();														// UI 삭제 이벤트 진행
	void ProgressShowUI();															// UI 표시 변경 이벤트 진행

	void ProgressChangeScene();														// 씬 전환 이벤트 진행

private:
	queue<pair<Scene*, GameObject*>>					addGameObjectQueue;			// 게임오브젝트 추가 이벤트 보관
	queue<pair<GameObject*, Component<GameObject>*>>	addChildQueue;				// 컴포넌트 추가 이벤트 보관
	queue<pair<Scene*, Component<GameObject>*>>		deleteObjectQueue;			// 오브젝트 삭제 이벤트 보관

	queue<pair<Scene*, UIBase*>>							addUIQueue;					// UI 추가 이벤트 보관
	queue<pair<UIBase*, UIBase*>>								addChildUIQueue;			// 자식UI 추가 이벤트 보관
	queue<pair<Scene*, UIBase*>>							deleteUIQueue;				// UI 삭제 이벤트 보관
	queue<pair<UIBase*, bool>>								showUIQueue;				// UI 표시 변경 이벤트 보관

	pair<int, float>*									changeSceneEvent;			// 씬 전환 이벤트 보관 변수
};

#define WORLD		WorldManager::GetInstance()

