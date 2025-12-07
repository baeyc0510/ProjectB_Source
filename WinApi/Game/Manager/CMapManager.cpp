#include "pch.h"
#include "CMapManager.h"

#include "Game/Object/Character/CEnemy.h"
#include "Game/Object/Character/CharacterFactory.h"
#include "Game/Object/World/CGround.h"
#include "Game/Object/World/CLadder.h"
#include "Game/Object/World/CLedge.h"
#include "Game/Object/World/CPlatform.h"

CMapManager::CMapManager()
	: currentMap(nullptr)
	, virtualCenter(0, 0)
{
}

CMapManager::~CMapManager()
{
	Release();
}

void CMapManager::Init()
{
	// 가상 해상도 중심 캐싱
	virtualCenter = SINGLE(CEngine)->GetVirtualSize() * 0.5f;
}

void CMapManager::Release()
{
	UnloadMap();
}

void CMapManager::LoadMap(const wstring& mapPath)
{
	currentMap = FindMap(mapPath);
	if (!currentMap)
	{
		currentMap = new CMap();
		currentMap->Load(mapPath);
		mapCache[mapPath] = currentMap;
	}
}

void CMapManager::UnloadMap()
{
	for (auto it = mapCache.begin(); it != mapCache.end(); ++it)
	{
		if (CMap* map = it->second)
		{
			map->Release();
			delete map;
		}
	}
	mapCache.clear();
	currentMap = nullptr;
}

CMap* CMapManager::FindMap(const wstring& mapPath)
{
	auto found = mapCache.find(mapPath);
	if (found != mapCache.end())
	{
		return found->second;
	}
	
	return nullptr;
}

void CMapManager::RenderBackground()
{
	if (!currentMap)
		return;

	Vec2 cameraPos = CAMERA->GetLookAt();
	currentMap->RenderBackground(cameraPos);
}

void CMapManager::RenderForeground()
{
	if (!currentMap)
		return;

	Vec2 cameraPos = CAMERA->GetLookAt();
	currentMap->RenderForeground(cameraPos);
}

Vec2 CMapManager::WorldToPixel(Vec2 world) const
{
	return world + virtualCenter;
}

Vec2 CMapManager::PixelToWorld(Vec2 pixel) const
{
	return pixel - virtualCenter;
}

Vec2 CMapManager::WorldToPixel(float x, float y) const
{
	return Vec2(x + virtualCenter.x, y + virtualCenter.y);
}

Vec2 CMapManager::PixelToWorld(float x, float y) const
{
	return Vec2(x - virtualCenter.x, y - virtualCenter.y);
}

Vec2 CMapManager::GetPlayerSpawn(int spawnId) const
{
	if (!currentMap)
		return Vec2(0, 0);
	return currentMap->GetSpawnPoint(spawnId);
}

Rect CMapManager::GetBounds() const
{
	if (!currentMap)
		return Rect();
	return currentMap->GetBounds();
}

const vector<CheckpointData>& CMapManager::GetCheckpoints() const
{
	static vector<CheckpointData> empty;
	if (!currentMap)
		return empty;
	return currentMap->GetCheckpoints();
}

CheckpointData* CMapManager::GetCheckpoint(int id)
{
	if (!currentMap)
		return nullptr;
	return currentMap->GetCheckpoint(id);
}

void CMapManager::ActivateCheckpoint(int id)
{
	if (currentMap)
		currentMap->ActivateCheckpoint(id);
}

const vector<SceneTransitionData>& CMapManager::GetTransitions() const
{
	static vector<SceneTransitionData> empty;
	if (!currentMap)
		return empty;
	return currentMap->GetTransitions();
}

void CMapManager::CreateWorldColliders(CScene* scene)
{
	if (!scene || !currentMap)
		return;

	DestroyWorldColliders();

	// 박스 콜라이더 생성
	for (const auto& box : currentMap->GetBoxColliders())
	{
		Vec2 center(box.rect.x + box.rect.w * 0.5f, box.rect.y + box.rect.h * 0.5f);
		Vec2 size(box.rect.w, box.rect.h);

		CGameObject* obj = nullptr;

		if (box.HasTag("Ladder"))
		{
			CLadder* ladder = new CLadder();
			ladder->SetBoxCollider(center, size);
			obj = ladder;
		}
		else if (box.HasTag("Platform"))
		{
			CPlatform* platform = new CPlatform();
			platform->SetBoxCollider(center, size);
			obj = platform;
		}
		else if (box.HasTag("Solid"))
		{
			CGround* ground = new CGround();
			ground->SetBoxCollider(center, size);
			obj = ground;
		}
		else if (box.HasTag("Ledge"))
		{
			CLedge* ledge = new CLedge();
			ledge->SetBoxCollider(center, size);
			ledge->SetCliffDirection(box.cliffDirection);
			obj = ledge;
		}

		scene->AddGameObject(obj);
		worldColliders.push_back(obj);
	}

	// 슬로프 콜라이더 생성
	for (const auto& slope : currentMap->GetSlopeColliders())
	{
		CGameObject* obj = nullptr;

		if (slope.HasTag("Platform"))
		{
			CPlatform* platform = new CPlatform();
			platform->SetLineCollider(slope.start, slope.end);
			obj = platform;
		}
		else
		{
			CGround* ground = new CGround();
			ground->SetLineCollider(slope.start, slope.end);
			obj = ground;
		}

		scene->AddGameObject(obj);
		worldColliders.push_back(obj);
	}
}

void CMapManager::DestroyWorldColliders()
{
	worldColliders.clear();
}

void CMapManager::CreateWorldCharacters(CScene* scene)
{
	if (!scene || !currentMap)
		return;
	
	for (const auto& objData : currentMap->GetWorldObjects())
	{
		if (CEnemy* enemy = CharacterFactory::CreateEnemy(objData.name))
		{
			enemy->SetPos(PixelToWorld(objData.pos));
			scene->AddGameObject(enemy);
		}
	}
}
