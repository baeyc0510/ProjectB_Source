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
	spawnPoints.clear();
	checkpoints.clear();
	transitions.clear();
	boxColliders.clear();
	slopeColliders.clear();
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
					metaMap.Load(metaPath);
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

// ============ 지형 충돌 체크 구현 ============

bool CMap::CheckGroundCollision(const Vec2& footPos, float halfWidth, float& outGroundY, bool& outOnSlope) const
{
	outOnSlope = false;
	float closestGroundY = 99999.f;
	bool foundGround = false;

	// 박스 콜라이더 체크 (Solid 또는 OneWay 태그)
	for (const auto& box : boxColliders)
	{
		if (!box.HasTag("Solid") && !box.HasTag("OneWay"))
			continue;

		float boxTop = box.rect.y;
		float boxBottom = box.rect.y + box.rect.h;
		float boxLeft = box.rect.x;
		float boxRight = box.rect.x + box.rect.w;

		// 발이 박스 X 범위 내에 있는지
		if (footPos.x + halfWidth < boxLeft || footPos.x - halfWidth > boxRight)
			continue;

		// 발이 박스 상단 근처에 있거나 박스 안에 들어갔는지 체크
		// (발 위치가 boxTop 위에서 조금 아래까지 허용)
		float aboveTolerance = 5.f;   // 위쪽 허용 오차 (착지 감지용)
		float belowTolerance = 20.f;  // 아래쪽 허용 오차 (관통 보정용)

		if (footPos.y >= boxTop - aboveTolerance && footPos.y <= boxTop + belowTolerance)
		{
			if (boxTop < closestGroundY)
			{
				closestGroundY = boxTop;
				foundGround = true;
			}
		}
	}

	// 슬로프 콜라이더 체크
	for (const auto& slope : slopeColliders)
	{
		if (!slope.HasTag("Solid"))
			continue;

		if (!slope.IsInRange(footPos.x))
			continue;

		float slopeY = slope.GetYAt(footPos.x);
		float aboveTolerance = 5.f;
		float belowTolerance = 20.f;

		if (footPos.y >= slopeY - aboveTolerance && footPos.y <= slopeY + belowTolerance)
		{
			if (slopeY < closestGroundY)
			{
				closestGroundY = slopeY;
				foundGround = true;
				outOnSlope = true;
			}
		}
	}

	outGroundY = closestGroundY;
	return foundGround;
}

bool CMap::CheckWallCollision(const Vec2& bodyPos, float halfWidth, float halfHeight, int direction, float& outWallX) const
{
	float checkX = bodyPos.x + (halfWidth + 1.f) * direction;
	float bodyTop = bodyPos.y - halfHeight;
	float bodyBottom = bodyPos.y + halfHeight;

	for (const auto& box : boxColliders)
	{
		if (!box.HasTag("Solid"))
			continue;

		float boxTop = box.rect.y;
		float boxBottom = box.rect.y + box.rect.h;
		float boxLeft = box.rect.x;
		float boxRight = box.rect.x + box.rect.w;

		// 세로 범위가 겹치는지
		if (bodyBottom < boxTop || bodyTop > boxBottom)
			continue;

		// 벽과의 충돌 체크
		if (direction > 0)  // 오른쪽 체크
		{
			if (checkX >= boxLeft && checkX <= boxRight)
			{
				outWallX = boxLeft;
				return true;
			}
		}
		else  // 왼쪽 체크
		{
			if (checkX >= boxLeft && checkX <= boxRight)
			{
				outWallX = boxRight;
				return true;
			}
		}
	}

	return false;
}

bool CMap::CheckCeilingCollision(const Vec2& headPos, float halfWidth, float& outCeilingY) const
{
	float closestCeilingY = -99999.f;
	bool foundCeiling = false;

	for (const auto& box : boxColliders)
	{
		if (!box.HasTag("Solid"))
			continue;

		float boxTop = box.rect.y;
		float boxBottom = box.rect.y + box.rect.h;
		float boxLeft = box.rect.x;
		float boxRight = box.rect.x + box.rect.w;

		// 머리가 박스 X 범위 내에 있는지
		if (headPos.x + halfWidth < boxLeft || headPos.x - halfWidth > boxRight)
			continue;

		// 머리가 박스 하단에 부딪히는지
		float tolerance = 5.f;
		if (headPos.y >= boxBottom - tolerance && headPos.y <= boxBottom + tolerance)
		{
			if (boxBottom > closestCeilingY)
			{
				closestCeilingY = boxBottom;
				foundCeiling = true;
			}
		}
	}

	outCeilingY = closestCeilingY;
	return foundCeiling;
}

float CMap::GetGroundYAt(float x, float startY, float maxDistance) const
{
	float groundY = startY + maxDistance;  // 기본값: 매우 아래

	// 박스 콜라이더에서 바닥 찾기
	for (const auto& box : boxColliders)
	{
		if (!box.HasTag("Solid") && !box.HasTag("OneWay"))
			continue;

		float boxTop = box.rect.y;
		float boxLeft = box.rect.x;
		float boxRight = box.rect.x + box.rect.w;

		if (x < boxLeft || x > boxRight)
			continue;

		// 시작 위치보다 아래에 있고, 현재 찾은 바닥보다 위에 있는 경우
		if (boxTop > startY && boxTop < groundY)
		{
			groundY = boxTop;
		}
	}

	// 슬로프 콜라이더에서 바닥 찾기
	for (const auto& slope : slopeColliders)
	{
		if (!slope.HasTag("Solid"))
			continue;

		if (!slope.IsInRange(x))
			continue;

		float slopeY = slope.GetYAt(x);
		if (slopeY > startY && slopeY < groundY)
		{
			groundY = slopeY;
		}
	}

	return groundY;
}

void CMap::RenderColliderDebug() const
{
	// 박스 콜라이더 렌더링
	for (const auto& box : boxColliders)
	{
		Vec2 topLeft = CAMERA->WorldToScreenPoint(Vec2(box.rect.x, box.rect.y));
		Vec2 bottomRight = CAMERA->WorldToScreenPoint(Vec2(box.rect.x + box.rect.w, box.rect.y + box.rect.h));

		if (box.HasTag("Solid"))
			RENDER->SetPen(PenType::Solid, RGB(255, 80, 80));
		else if (box.HasTag("OneWay"))
			RENDER->SetPen(PenType::Solid, RGB(80, 255, 80));
		else
			RENDER->SetPen(PenType::Solid, RGB(150, 150, 150));

		RENDER->SetBrush(BrushType::Null);
		RENDER->Rect(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
	}

	// 슬로프 콜라이더 렌더링
	for (const auto& slope : slopeColliders)
	{
		Vec2 start = CAMERA->WorldToScreenPoint(slope.start);
		Vec2 end = CAMERA->WorldToScreenPoint(slope.end);

		RENDER->SetPen(PenType::Solid, RGB(80, 200, 255));
		RENDER->Line(start.x, start.y, end.x, end.y);

		// 끝점 표시
		RENDER->SetBrush(BrushType::Solid, RGB(80, 200, 255));
		RENDER->Ellipse(start.x - 4, start.y - 4, start.x + 4, start.y + 4);
		RENDER->Ellipse(end.x - 4, end.y - 4, end.x + 4, end.y + 4);
	}

	RENDER->SetPen();
	RENDER->SetBrush();
}
