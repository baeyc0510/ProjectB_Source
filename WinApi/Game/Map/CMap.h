#pragma once
#include "CMapLayer.h"
#include "CMetaMap.h"
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

	// 콜라이더
	const vector<BoxColliderData>& GetBoxColliders() const { return boxColliders; }
	const vector<SlopeColliderData>& GetSlopeColliders() const { return slopeColliders; }

	// 태그로 콜라이더 필터링
	vector<const BoxColliderData*> GetBoxCollidersWithTag(const string& tag) const;
	vector<const SlopeColliderData*> GetSlopeCollidersWithTag(const string& tag) const;

	// ============ 지형 충돌 체크 ============

	// 바닥 충돌 체크 - 특정 위치에서 바닥과 충돌하는지, 충돌 시 보정된 Y 반환
	// footPos: 발 위치, halfWidth: 캐릭터 반폭
	// outGroundY: 충돌 시 바닥 Y 좌표, outOnSlope: 슬로프 위인지
	bool CheckGroundCollision(const Vec2& footPos, float halfWidth, float& outGroundY, bool& outOnSlope) const;

	// 벽 충돌 체크 - 특정 위치에서 벽과 충돌하는지
	// bodyPos: 몸통 중심, halfWidth: 캐릭터 반폭, halfHeight: 캐릭터 반높이
	// direction: 체크 방향 (-1: 왼쪽, 1: 오른쪽)
	// outWallX: 충돌 시 벽 X 좌표
	bool CheckWallCollision(const Vec2& bodyPos, float halfWidth, float halfHeight, int direction, float& outWallX) const;

	// 천장 충돌 체크
	// headPos: 머리 위치, halfWidth: 캐릭터 반폭
	// outCeilingY: 충돌 시 천장 Y 좌표
	bool CheckCeilingCollision(const Vec2& headPos, float halfWidth, float& outCeilingY) const;

	// X 위치에서 바닥 Y 좌표 얻기 (바닥이 없으면 매우 큰 값 반환)
	float GetGroundYAt(float x, float startY, float maxDistance = 500.f) const;

	// 콜라이더 디버그 렌더링
	void RenderColliderDebug() const;

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
	vector<BoxColliderData> boxColliders;
	vector<SlopeColliderData> slopeColliders;

	wstring mapDirectory;	// JSON 파일이 있는 디렉토리
};
