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

	metaMap.Release();
	checkpoints.clear();
	transitions.clear();
}

void CMap::RenderBackground(Vec2 cameraPos)
{
	// 배경 레이어들 렌더링 (뒤에서 앞으로)
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
	// 이미지 경로는 resourceFolder (Resources/) 기준 상대경로
	// isMain 필드로 메인 레이어 결정, 메인 이전은 background, 이후는 foreground
	if (mapData.contains("layers"))
	{
		// 1단계: 모든 레이어 데이터와 메인 레이어 인덱스 찾기
		const auto& layers = mapData["layers"];
		int mainLayerIndex = -1;

		for (size_t i = 0; i < layers.size(); ++i)
		{
			if (layers[i].value("isMain", false))
			{
				mainLayerIndex = (int)i;
				break;
			}
		}

		// 2단계: 레이어 분류
		bool isFirstBackground = true;  // 첫 번째 배경만 BitBlt 사용
		for (size_t i = 0; i < layers.size(); ++i)
		{
			const auto& layerData = layers[i];
			string imageName = layerData["image"];
			float parallax = layerData.value("parallax", 1.0f);
			float offsetX = layerData.value("offsetX", 0.0f);
			float offsetY = layerData.value("offsetY", 0.0f);
			// JSON에서 transparent 설정 가능, 기본값은 레이어 타입에 따라 결정
			bool hasTransparentField = layerData.contains("transparent");

			wstring imagePath = StringToWString(imageName);

			if ((int)i == mainLayerIndex)
			{
				// 메인 레이어 - 기본 투명 처리 true
				bool transparent = hasTransparentField ? layerData["transparent"].get<bool>() : true;
				mainLayer.Load(imagePath, parallax, Vec2(offsetX, offsetY), transparent);

				// 메타 이미지 로드 (있으면)
				if (layerData.contains("meta"))
				{
					string metaName = layerData["meta"];
					wstring metaPath = StringToWString(metaName);
					metaMap.Load(metaPath);
				}
			}
			else if (mainLayerIndex < 0 || (int)i < mainLayerIndex)
			{
				// 배경 레이어 - 첫 번째만 BitBlt, 나머지는 TransparentBlt
				bool transparent = hasTransparentField ? layerData["transparent"].get<bool>() : !isFirstBackground;
				CMapLayer layer;
				layer.Load(imagePath, parallax, Vec2(offsetX, offsetY), transparent);
				backgroundLayers.push_back(std::move(layer));
				isFirstBackground = false;
			}
			else
			{
				// 전경 레이어 - 기본 투명 처리 true
				bool transparent = hasTransparentField ? layerData["transparent"].get<bool>() : true;
				CMapLayer layer;
				layer.Load(imagePath, parallax, Vec2(offsetX, offsetY), transparent);
				foregroundLayers.push_back(std::move(layer));
			}
		}
	}

	// 플레이어 스폰 위치 (픽셀 좌표 → 월드 좌표)
	// 맵 에디터는 픽셀 좌표를 저장하고, 게임은 월드 좌표를 사용
	// 변환: worldPos = pixelPos - virtualCenter
	Vec2 virtualCenter = SINGLE(CEngine)->GetVirtualSize() * 0.5f;
	if (mapData.contains("playerSpawn"))
	{
		playerSpawn.x = mapData["playerSpawn"][0];
		playerSpawn.y = mapData["playerSpawn"][1];
		playerSpawn = playerSpawn - virtualCenter;
	}

	// 카메라 바운드 (픽셀 좌표 → 월드 좌표)
	if (mapData.contains("bounds"))
	{
		bounds.x = mapData["bounds"][0];
		bounds.y = mapData["bounds"][1];
		bounds.w = mapData["bounds"][2];
		bounds.h = mapData["bounds"][3];
		bounds.x -= virtualCenter.x;
		bounds.y -= virtualCenter.y;
	}

	// 체크포인트 (픽셀 좌표 → 월드 좌표)
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

	// 씬 전환 트리거 (픽셀 좌표 → 월드 좌표)
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
