#pragma once
#include "Game/Map/Map.h"

class Scene;

class MapManager : public SingleTon<MapManager>
{
	friend SingleTon<MapManager>;

private:
	MapManager();
	~MapManager();

public:
	void Init();
	void Release();

	// 맵 관리
	Map* LoadMap(const wstring& mapPath);			// 맵 로드 및 캐싱
	void SetActiveMap(const wstring& mapPath);		// 활성 맵 설정
	void UnloadMap();
	bool IsMapLoaded() const { return activeMap != nullptr; }

	Map* FindMap(const wstring& mapPath);
	
	// 렌더링
	void RenderBackground();
	void RenderForeground();

	// 좌표 변환
	// World 좌표: 게임 로직 기준
	// Pixel 좌표: 맵/메타맵 기준, 좌상단이 (0, 0)
	Vec2 WorldToPixel(Vec2 world) const;
	Vec2 PixelToWorld(Vec2 pixel) const;
	Vec2 WorldToPixel(float x, float y) const;
	Vec2 PixelToWorld(float x, float y) const;

	// 맵 정보
	Vec2 GetPlayerSpawn(int spawnId = 0) const;
	Rect GetBounds() const;
	const vector<CheckpointData>& GetCheckpoints() const;
	CheckpointData* GetCheckpoint(int id);
	void ActivateCheckpoint(int id);
	const vector<SceneTransitionData>& GetTransitions() const;
	
	Map* GetCurrentMap() { return currentMap; }
	Map* GetActiveMap() { return activeMap; }

	// 월드 콜라이더/오브젝트 생성
	void CreateWorldColliders(Scene* scene);
	void DestroyWorldColliders();
	void CreateWorldCharacters(Scene* scene);
	void CreateWorldCheckpoints(Scene* scene);

private:
	Map* currentMap;		// 현재 작업 중인 맵 (로드/스폰용)
	Map* activeMap;			// 활성 맵 (렌더링용)
	Vec2 virtualCenter;
	vector<GameObject*> worldColliders;
	unordered_map<wstring, Map*> mapCache;
};

#define MAP		MapManager::GetInstance()
