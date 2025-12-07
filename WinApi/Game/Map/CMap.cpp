#include "pch.h"
#include "CMap.h"
#include <fstream>

using json = nlohmann::json;

CMap::CMap()
{
}

CMap::~CMap()
{
	Release();
}

void CMap::Load(const wstring& mapJsonPath)
{
	Release();
	mapPath = mapJsonPath;
	LoadFromJson(mapJsonPath);
}

void CMap::Release()
{
	for (auto& layer : backgroundLayers)
		layer.Release();
	backgroundLayers.clear();

	mainLayer.Release();

	for (auto& layer : foregroundLayers)
		layer.Release();
	foregroundLayers.clear();

	spawnPoints.clear();
	checkpoints.clear();
	transitions.clear();
	boxColliders.clear();
	slopeColliders.clear();
	mapPath = L"";
}

void CMap::RenderBackground(Vec2 cameraPos)
{
	// 배경 레이어들 렌더링 (뒤(낮은 인덱스)-> 앞(높은 인덱스))
	for (auto& layer : backgroundLayers)
	{
		layer.Render(cameraPos);
	}

	// 메인 레이어 렌더링
	mainLayer.Render(cameraPos);
}

void CMap::RenderForeground(Vec2 cameraPos)
{
	// 전경 레이어들 렌더링
	for (auto& layer : foregroundLayers)
	{
		layer.Render(cameraPos);
	}
}

Vec2 CMap::GetSpawnPoint(int spawnId) const
{
	if (spawnId < 0 || spawnId >= spawnPoints.size())
	{
		Logger::Error(L"Invalid spawnId: " + spawnId);
		return Vec2();
	}
	
	return spawnPoints[spawnId];
}

CheckpointData* CMap::GetCheckpoint(int id)
{
	for (auto& cp : checkpoints)
	{
		if (cp.id == id)
			return &cp;
	}
	return nullptr;
}

void CMap::ActivateCheckpoint(int id)
{
	if (CheckpointData* cp = GetCheckpoint(id))
	{
		cp->activated = true;
	}
}

const SceneTransitionData* CMap::GetTransitionAt(Vec2 pos) const
{
	for (const auto& transition : transitions)
	{
		if (pos.x >= transition.rect.x &&
			pos.x <= transition.rect.x + transition.rect.w &&
			pos.y >= transition.rect.y &&
			pos.y <= transition.rect.y + transition.rect.h)
		{
			return &transition;
		}
	}
	return nullptr;
}

void CMap::LoadFromJson(const wstring& jsonPath)
{
	// resourceFolder 기준 절대 경로로 변환
	wstring fullPath = CResourceManager::GetInstance()->GetResourceFolder() + jsonPath;
	mapDirectory = GetMapDirectory(fullPath);

	// JSON 파일 읽기
	string narrowPath = WStringToString(fullPath);
	ifstream file(narrowPath);
	if (!file.is_open())
	{
		Logger::Error(L"Failed to open map file: " + jsonPath);
		return;
	}

	json mapData;
	file >> mapData;
	file.close();

	// 레이어 로드
	if (mapData.contains("layers"))
	{
		const auto& layers = mapData["layers"];
		int mainLayerIndex = -1;

		for (size_t i = 0; i < layers.size(); ++i)
		{
			// 메인레이어 찾기
			if (layers[i].value("isMain", false))
			{
				mainLayerIndex = (int)i;
				break;
			}
		}

		bool isFirstBackground = true;
		for (size_t i = 0; i < layers.size(); ++i)
		{
			const auto& layerData = layers[i];
			string imageName = layerData["image"];
			float parallax = layerData.value("parallax", 1.0f);
			float offsetX = layerData.value("offsetX", 0.0f);
			float offsetY = layerData.value("offsetY", 0.0f);
			bool bHasTransparentField = layerData.contains("transparent");

			wstring imagePath = StringToWString(imageName);

			if ((int)i == mainLayerIndex)
			{
				// 메인 레이어 - 기본 투명 처리 true
				bool bIsTransparent = bHasTransparentField ? layerData["transparent"].get<bool>() : true;
				mainLayer.Load(imagePath, parallax, Vec2(offsetX, offsetY), bIsTransparent);

				// 메타 이미지 로드
				if (layerData.contains("meta"))
				{
					string metaName = layerData["meta"];
					wstring metaPath = StringToWString(metaName);
				}
			}
			else if (mainLayerIndex < 0 || (int)i < mainLayerIndex)
			{
				// 배경 레이어 - 첫 번째만 BitBlt, 나머지는 TransparentBlt
				bool bIsTransparent = bHasTransparentField ? layerData["transparent"].get<bool>() : !isFirstBackground;
				CMapLayer layer;
				layer.Load(imagePath, parallax, Vec2(offsetX, offsetY), bIsTransparent);
				backgroundLayers.push_back(std::move(layer));
				isFirstBackground = false;
			}
			else
			{
				// 전경 레이어 - 기본 투명 처리 true
				bool bIsTransparent = bHasTransparentField ? layerData["transparent"].get<bool>() : true;
				CMapLayer layer;
				layer.Load(imagePath, parallax, Vec2(offsetX, offsetY), bIsTransparent);
				foregroundLayers.push_back(std::move(layer));
			}
		}
	}

	// 스폰 위치
	// 변환: worldPos = pixelPos - virtualCenter
	Vec2 virtualCenter = SINGLE(CEngine)->GetVirtualSize() * 0.5f;
	if (mapData.contains("spawnPoints"))
	{
		for (auto& spawnData : mapData["spawnPoints"])
		{
			Vec2 spawnPoint(spawnData["pos"][0],spawnData["pos"][1]);
			spawnPoint = spawnPoint - virtualCenter;
			spawnPoints.push_back(spawnPoint);
		}
	}

	// 카메라 바운드
	if (mapData.contains("bounds"))
	{
		bounds.x = mapData["bounds"][0];
		bounds.y = mapData["bounds"][1];
		bounds.w = mapData["bounds"][2];
		bounds.h = mapData["bounds"][3];
		bounds.x -= virtualCenter.x;
		bounds.y -= virtualCenter.y;
	}

	// 체크포인트
	if (mapData.contains("checkpoints"))
	{
		for (const auto& cpData : mapData["checkpoints"])
		{
			CheckpointData cp;
			cp.id = cpData["id"];
			cp.pos.x = cpData["pos"][0];
			cp.pos.y = cpData["pos"][1];
			cp.pos = cp.pos - virtualCenter;
			cp.activated = false;
			checkpoints.push_back(cp);
		}
	}

	// 씬 전환 트리거
	if (mapData.contains("transitions"))
	{
		for (const auto& transData : mapData["transitions"])
		{
			SceneTransitionData trans;
			trans.rect.x = transData["rect"][0];
			trans.rect.y = transData["rect"][1];
			trans.rect.w = transData["rect"][2];
			trans.rect.h = transData["rect"][3];
			trans.rect.x -= virtualCenter.x;
			trans.rect.y -= virtualCenter.y;
			trans.targetScene = StringToWString(transData["target"]);
			trans.spawnId = transData.value("spawnId", 0);
			transitions.push_back(trans);
		}
	}

	// 박스 콜라이더
	if (mapData.contains("boxColliders"))
	{
		for (const auto& boxData : mapData["boxColliders"])
		{
			BoxColliderData box;
			box.rect.x = boxData["rect"][0];
			box.rect.y = boxData["rect"][1];
			box.rect.w = boxData["rect"][2];
			box.rect.h = boxData["rect"][3];
			box.rect.x -= virtualCenter.x;
			box.rect.y -= virtualCenter.y;

			if (boxData.contains("tags"))
			{
				for (const auto& tag : boxData["tags"])
				{
					box.tags.push_back(tag.get<string>());
				}
			}
			else
			{
				box.tags.push_back("Solid");
			}

			// Ledge의 cliffDirection 로드
			if (boxData.contains("cliffDirection"))
			{
				box.cliffDirection = boxData["cliffDirection"].get<int>();
			}

			boxColliders.push_back(box);
		}
	}

	// 슬로프 콜라이더
	if (mapData.contains("slopeColliders"))
	{
		for (const auto& slopeData : mapData["slopeColliders"])
		{
			SlopeColliderData slope;
			slope.start.x = slopeData["points"][0];
			slope.start.y = slopeData["points"][1];
			slope.end.x = slopeData["points"][2];
			slope.end.y = slopeData["points"][3];
			slope.start.x -= virtualCenter.x;
			slope.start.y -= virtualCenter.y;
			slope.end.x -= virtualCenter.x;
			slope.end.y -= virtualCenter.y;

			if (slopeData.contains("tags"))
			{
				for (const auto& tag : slopeData["tags"])
				{
					slope.tags.push_back(tag.get<string>());
				}
			}
			else
			{
				slope.tags.push_back("Solid");
			}
			slopeColliders.push_back(slope);
		}
	}
	
	// 월드 오브젝트
	if (mapData.contains("objects"))
	{
		for (const auto& objectData : mapData["objects"])
		{
			WorldObjectData data;
			data.name = objectData["name"].get<string>();
			data.pos.x = objectData["pos"][0];
			data.pos.y = objectData["pos"][1];
			
			worldObjects.push_back(data);
		}
	}
}

wstring CMap::GetMapDirectory(const wstring& jsonPath)
{
	size_t lastSlash = jsonPath.find_last_of(L"/\\");
	if (lastSlash != wstring::npos)
	{
		return jsonPath.substr(0, lastSlash + 1);
	}
	return L"";
}

vector<const BoxColliderData*> CMap::GetBoxCollidersWithTag(const string& tag) const
{
	vector<const BoxColliderData*> result;
	for (const auto& box : boxColliders)
	{
		if (box.HasTag(tag))
			result.push_back(&box);
	}
	return result;
}

vector<const SlopeColliderData*> CMap::GetSlopeCollidersWithTag(const string& tag) const
{
	vector<const SlopeColliderData*> result;
	for (const auto& slope : slopeColliders)
	{
		if (slope.HasTag(tag))
			result.push_back(&slope);
	}
	return result;
}

