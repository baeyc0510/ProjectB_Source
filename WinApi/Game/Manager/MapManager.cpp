#include "pch.h"
#include "MapManager.h"

#include "Game/Object/Character/Enemy.h"
#include "Game/Object/World/Ground.h"
#include "Game/Object/World/LadderCollider.h"
#include "Game/Object/World/LedgeCollider.h"
#include "Game/Object/World/Platform.h"
#include "Game/Util/CharacterFactory.h"

MapManager::MapManager()
	: currentMap(nullptr)
	, virtualCenter(0, 0)
{
}

MapManager::~MapManager()
{
	Release();
}

void MapManager::Init()
{
	// 가상 해상도 중심 캐싱
	virtualCenter = SINGLE(EngineInstance)->GetVirtualSize() * 0.5f;
}

void MapManager::Release()
{
	UnloadMap();
}

void MapManager::LoadMap(const wstring& mapPath)
{
	currentMap = FindMap(mapPath);
	if (!currentMap)
	{
		currentMap = new Map();
		currentMap->Load(mapPath);
		mapCache[mapPath] = currentMap;
	}
}

void MapManager::UnloadMap()
{
	for (auto it = mapCache.begin(); it != mapCache.end(); ++it)
	{
		if (Map* map = it->second)
		{
			map->Release();
			delete map;
		}
	}
	mapCache.clear();
	currentMap = nullptr;
}

Map* MapManager::FindMap(const wstring& mapPath)
{
	auto found = mapCache.find(mapPath);
	if (found != mapCache.end())
	{
		return found->second;
	}
	
	return nullptr;
}

void MapManager::RenderBackground()
{
	if (!currentMap)
		return;

	Vec2 cameraPos = CAMERA->GetLookAt();
	currentMap->RenderBackground(cameraPos);
}

void MapManager::RenderForeground()
{
	if (!currentMap)
		return;

	Vec2 cameraPos = CAMERA->GetLookAt();
	currentMap->RenderForeground(cameraPos);
}

Vec2 MapManager::WorldToPixel(Vec2 world) const
{
	return world + virtualCenter;
}

Vec2 MapManager::PixelToWorld(Vec2 pixel) const
{
	return pixel - virtualCenter;
}

Vec2 MapManager::WorldToPixel(float x, float y) const
{
	return Vec2(x + virtualCenter.x, y + virtualCenter.y);
}

Vec2 MapManager::PixelToWorld(float x, float y) const
{
	return Vec2(x - virtualCenter.x, y - virtualCenter.y);
}

Vec2 MapManager::GetPlayerSpawn(int spawnId) const
{
	if (!currentMap)
		return Vec2(0, 0);
	return currentMap->GetSpawnPoint(spawnId);
}

Rect MapManager::GetBounds() const
{
	if (!currentMap)
		return Rect();
	return currentMap->GetBounds();
}

const vector<CheckpointData>& MapManager::GetCheckpoints() const
{
	static vector<CheckpointData> empty;
	if (!currentMap)
		return empty;
	return currentMap->GetCheckpoints();
}

CheckpointData* MapManager::GetCheckpoint(int id)
{
	if (!currentMap)
		return nullptr;
	return currentMap->GetCheckpoint(id);
}

void MapManager::ActivateCheckpoint(int id)
{
	if (currentMap)
		currentMap->ActivateCheckpoint(id);
}

const vector<SceneTransitionData>& MapManager::GetTransitions() const
{
	static vector<SceneTransitionData> empty;
	if (!currentMap)
		return empty;
	return currentMap->GetTransitions();
}

void MapManager::CreateWorldColliders(Scene* scene)
{
	if (!scene || !currentMap)
		return;

	DestroyWorldColliders();

	// 박스 콜라이더 생성
	for (const auto& box : currentMap->GetBoxColliders())
	{
		Vec2 center(box.rect.x + box.rect.w * 0.5f, box.rect.y + box.rect.h * 0.5f);
		Vec2 size(box.rect.w, box.rect.h);

		GameObject* obj = nullptr;

		if (box.HasTag("Ladder"))
		{
			LadderCollider* ladder = new LadderCollider();
			ladder->SetBoxCollider(center, size);
			obj = ladder;
		}
		else if (box.HasTag("Platform"))
		{
			Platform* platform = new Platform();
			platform->SetBoxCollider(center, size);
			obj = platform;
		}
		else if (box.HasTag("Solid"))
		{
			Ground* ground = new Ground();
			ground->SetBoxCollider(center, size);
			obj = ground;
		}
		else if (box.HasTag("Ledge"))
		{
			LedgeCollider* ledge = new LedgeCollider();
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
		GameObject* obj = nullptr;

		if (slope.HasTag("Platform"))
		{
			Platform* platform = new Platform();
			platform->SetLineCollider(slope.start, slope.end);
			obj = platform;
		}
		else
		{
			Ground* ground = new Ground();
			ground->SetLineCollider(slope.start, slope.end);
			obj = ground;
		}

		scene->AddGameObject(obj);
		worldColliders.push_back(obj);
	}
}

void MapManager::DestroyWorldColliders()
{
	worldColliders.clear();
}

void MapManager::CreateWorldCharacters(Scene* scene)
{
	if (!scene || !currentMap)
		return;
	
	for (const auto& objData : currentMap->GetWorldObjects())
	{
		if (Character* character = CharacterFactory::CreateCharacter(objData.name))
		{
			character->SetPos(PixelToWorld(objData.pos));
			scene->AddGameObject(character);
		}
	}
}
