#pragma once
#include "CMapLayer.h"
#include "CMetaMap.h"

struct CheckpointData
{
	int id;
	Vec2 pos;
	bool activated = false;
};

struct SceneTransitionData
{
	Rect rect;
	wstring targetScene;
	int spawnId;
};

class CMap
{
public:
	CMap();
	~CMap();

	// JSON 파일로부터 맵 로드
	void Load(const wstring& mapJsonPath);
	void Release();

	// 렌더링 (zOrder 순서대로)
	void RenderBackground(Vec2 cameraPos);	// Main 레이어 이전 (뒤)
	void RenderForeground(Vec2 cameraPos);	// Main 레이어 이후 (앞)

	// 메타맵 접근
	CMetaMap* GetMetaMap() { return &metaMap; }
	const CMetaMap* GetMetaMap() const { return &metaMap; }

	// 지형 조회
	ETerrain GetTerrain(Vec2 pos) const { return metaMap.GetTerrain(pos); }
	ETerrain GetTerrain(int x, int y) const { return metaMap.GetTerrain(x, y); }

	// 맵 정보
	Vec2 GetSpawnPoint(int spawnId) const;
	Rect GetBounds() const { return bounds; }

	// 체크포인트
	const vector<CheckpointData>& GetCheckpoints() const { return checkpoints; }
	CheckpointData* GetCheckpoint(int id);
	void ActivateCheckpoint(int id);

	// 씬 전환
	const vector<SceneTransitionData>& GetTransitions() const { return transitions; }
	const SceneTransitionData* GetTransitionAt(Vec2 pos) const;

private:
	void LoadFromJson(const wstring& jsonPath);
	wstring GetMapDirectory(const wstring& jsonPath);

private:
	vector<CMapLayer> backgroundLayers;
	CMapLayer mainLayer;
	vector<CMapLayer> foregroundLayers;

	CMetaMap metaMap;
	
	Rect bounds;
	vector<Vec2> spawnPoints;
	vector<CheckpointData> checkpoints;
	vector<SceneTransitionData> transitions;

	wstring mapDirectory;	// JSON 파일이 있는 디렉토리
};
