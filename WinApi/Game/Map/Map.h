#pragma once
#include "MapLayer.h"
#include <vector>
#include <string>

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

// 박스 콜라이더 데이터
struct BoxColliderData
{
	Rect rect;
	vector<string> tags;
	int cliffDirection = 0;  // Ledge용: -1=왼쪽 절벽, 0=양방향, 1=오른쪽 절벽

	bool HasTag(const string& tag) const
	{
		for (const auto& t : tags)
			if (t == tag) return true;
		return false;
	}
};

// 슬로프 콜라이더 데이터
struct SlopeColliderData
{
	Vec2 start;  // 시작점
	Vec2 end;    // 끝점
	vector<string> tags;

	bool HasTag(const string& tag) const
	{
		for (const auto& t : tags)
			if (t == tag) return true;
		return false;
	}

	// x 위치에서의 Y 높이 계산
	float GetYAt(float x) const
	{
		if (abs(end.x - start.x) < 0.001f) return start.y;
		float t = (x - start.x) / (end.x - start.x);
		t = max(0.0f, min(1.0f, t));
		return start.y + t * (end.y - start.y);
	}

	// x가 슬로프 범위 내인지 확인
	bool IsInRange(float x) const
	{
		float minX = min(start.x, end.x);
		float maxX = max(start.x, end.x);
		return x >= minX && x <= maxX;
	}
};

struct WorldObjectData
{
	Vec2 pos;
	string name;
};

class Map
{
public:
	Map();
	~Map();

	// JSON 파일로부터 맵 로드
	void Load(const wstring& mapJsonPath);
	void Release();

	// 렌더링 (zOrder 순서대로)
	void RenderBackground(Vec2 cameraPos);	// Main 레이어 이전 (뒤)
	void RenderForeground(Vec2 cameraPos);	// Main 레이어 이후 (앞)
	
	// 맵 정보
	wstring GetMapPath() const { return mapPath; }
	Vec2 GetSpawnPoint(int spawnId) const;
	Rect GetBounds() const { return bounds; }

	// 체크포인트
	const vector<CheckpointData>& GetCheckpoints() const { return checkpoints; }
	CheckpointData* GetCheckpoint(int id);
	void ActivateCheckpoint(int id);

	// 씬 전환
	const vector<SceneTransitionData>& GetTransitions() const { return transitions; }
	const SceneTransitionData* GetTransitionAt(Vec2 pos) const;

	// 콜라이더
	const vector<BoxColliderData>& GetBoxColliders() const { return boxColliders; }
	const vector<SlopeColliderData>& GetSlopeColliders() const { return slopeColliders; }

	// 월드 오브젝트
	const vector<WorldObjectData>& GetWorldObjects() const { return worldObjects; }
	
	// 태그로 콜라이더 필터링
	vector<const BoxColliderData*> GetBoxCollidersWithTag(const string& tag) const;
	vector<const SlopeColliderData*> GetSlopeCollidersWithTag(const string& tag) const;


private:
	void LoadFromJson(const wstring& jsonPath);
	wstring GetMapDirectory(const wstring& jsonPath);

private:
	wstring mapPath;
	vector<MapLayer> backgroundLayers;
	MapLayer mainLayer;
	vector<MapLayer> foregroundLayers;

	Rect bounds;
	vector<Vec2> spawnPoints;
	vector<CheckpointData> checkpoints;
	vector<SceneTransitionData> transitions;
	vector<BoxColliderData> boxColliders;
	vector<SlopeColliderData> slopeColliders;
	vector<WorldObjectData> worldObjects;

	wstring mapDirectory;	// JSON 파일이 있는 디렉토리
};
