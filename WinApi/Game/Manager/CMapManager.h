#pragma once
#include "Game/Map/CMap.h"

class CMapManager : public SingleTon<CMapManager>
{
	friend SingleTon<CMapManager>;

private:
	CMapManager();
	~CMapManager();

public:
	void Init();
	void Release();

	// 맵 관리
	void LoadMap(const wstring& mapPath);
	void UnloadMap();
	bool IsMapLoaded() const { return currentMap != nullptr; }

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

	// 메타맵 조회 (월드 좌표 사용)
	bool IsGroundAt(Vec2 worldPos) const;
	bool IsGroundAt(float worldX, float worldY) const;
	bool IsSolidAt(Vec2 worldPos) const;
	bool IsSolidAt(float worldX, float worldY) const;
	ETerrain GetTerrainAt(Vec2 worldPos) const;
	ETerrain GetTerrainAt(float worldX, float worldY) const;

	// 맵 정보
	Vec2 GetPlayerSpawn(int spawnId = 0) const;
	Rect GetBounds() const;
	const vector<CheckpointData>& GetCheckpoints() const;
	CheckpointData* GetCheckpoint(int id);
	void ActivateCheckpoint(int id);
	const vector<SceneTransitionData>& GetTransitions() const;
	
	CMap* GetCurrentMap() { return currentMap; }
	CMetaMap* GetMetaMap();

private:
	CMap* currentMap;
	Vec2 virtualCenter;		// 가상 해상도 중심
};

#define MAP		CMapManager::GetInstance()
