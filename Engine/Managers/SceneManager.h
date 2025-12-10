#pragma once

// 전방선언 : 클래스의 이름만으로 선언
class Scene;

class SceneManager : public SingleTon<SceneManager>
{
	friend SingleTon<SceneManager>;
private:
	SceneManager();
	virtual ~SceneManager();

public:
	void		Init();
	void		Update();
	void		Render();
	void		Release();

	Scene*		FindScene(int key);					// 씬 탐색
	void		AddScene(int key, Scene* scene);	// 씬 추가
	void		RemoveScene(int key);				// 씬 삭제
	void		ChangeScene(int key);				// 씬 전환
	void		SetStartScene(int key);				// 시작 씬 설정

	Scene*		GetCurScene();						// 현재 씬 반환

private:
	map<int, Scene*>		mapScene;				// 게임씬들을 보관하는 자료구조
	Scene*					curScene;				// 현재 게임씬
};

