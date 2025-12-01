#include "pch.h"
#include "CMapManager.h"

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
	UnloadMap();

	currentMap = new CMap();
	currentMap->Load(mapPath);
}

void CMapManager::UnloadMap()
{
	if (currentMap)
	{
		currentMap->Release();
		delete currentMap;
		currentMap = nullptr;
	}
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

bool CMapManager::IsGroundAt(Vec2 worldPos) const
{
	if (!currentMap || !currentMap->GetMetaMap()->IsLoaded())
		return false;

	Vec2 pixelPos = WorldToPixel(worldPos);
	return currentMap->GetMetaMap()->IsGroundAt((int)pixelPos.x, (int)pixelPos.y);
}

bool CMapManager::IsGroundAt(float worldX, float worldY) const
{
	return IsGroundAt(Vec2(worldX, worldY));
}

bool CMapManager::IsSolidAt(Vec2 worldPos) const
{
	if (!currentMap || !currentMap->GetMetaMap()->IsLoaded())
		return false;

	Vec2 pixelPos = WorldToPixel(worldPos);
	return currentMap->GetMetaMap()->IsSolid((int)pixelPos.x, (int)pixelPos.y);
}

bool CMapManager::IsSolidAt(float worldX, float worldY) const
{
	return IsSolidAt(Vec2(worldX, worldY));
}

ETerrain CMapManager::GetTerrainAt(Vec2 worldPos) const
{
	if (!currentMap || !currentMap->GetMetaMap()->IsLoaded())
		return ETerrain::Empty;

	Vec2 pixelPos = WorldToPixel(worldPos);
	return currentMap->GetMetaMap()->GetTerrain((int)pixelPos.x, (int)pixelPos.y);
}

ETerrain CMapManager::GetTerrainAt(float worldX, float worldY) const
{
	return GetTerrainAt(Vec2(worldX, worldY));
}

Vec2 CMapManager::GetPlayerSpawn() const
{
	if (!currentMap)
		return Vec2(0, 0);
	return currentMap->GetPlayerSpawn();
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

CMetaMap* CMapManager::GetMetaMap()
{
	if (!currentMap)
		return nullptr;
	return currentMap->GetMetaMap();
}
