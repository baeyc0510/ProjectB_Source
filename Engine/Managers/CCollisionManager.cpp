#include "pch.h"
#include "CCollisionManager.h"

CCollisionManager::CCollisionManager()
{
	ResetCheck();
}

CCollisionManager::~CCollisionManager()
{
}

void CCollisionManager::Init()
{
}

void CCollisionManager::Update()
{
	for (UINT left = 0; left < MAXLAYER; left++)
	{
		for (UINT right = left; right < MAXLAYER; right++)
		{
			if (layerMask[left][right])
			{
				CollisionUpdate(left, right);
			}
		}
	}
}

void CCollisionManager::Release()
{
}

void CCollisionManager::AddCollider(CCollider* collider)
{
	colliderList[collider->GetLayer()].push_back(collider);
}

void CCollisionManager::RemoveCollider(CCollider* collider)
{
	colliderList[collider->GetLayer()].remove(collider);
}

void CCollisionManager::CheckLayer(UINT left, UINT right)
{
	layerMask[left][right] = true;
	layerMask[right][left] = true;
}

void CCollisionManager::UnCheckLayer(UINT left, UINT right)
{
	layerMask[left][right] = false;
	layerMask[right][left] = false;
}

void CCollisionManager::ResetCheck()
{
	for (UINT i = 0; i < layerMask.size(); i++)
		layerMask[i].fill(false);
}

void CCollisionManager::CollisionUpdate(UINT left, UINT right)
{
	for (CCollider* leftCollider : colliderList[left])
	{
		for (CCollider* rightCollider : colliderList[right])
		{
			// 자기 자신과의 충돌을 무시
			if (leftCollider == rightCollider)
				continue;

			// 충돌체와 충돌체의 ID 확인
			UINT64 collisionID = CollisionID(leftCollider->GetID(), rightCollider->GetID());
			// 충돌 정보가 없었던 경우, 충돌하지 않은 상태를 추가
			if (prevCollision.find(collisionID) == prevCollision.end())
				prevCollision.insert(make_pair(collisionID, false));

			// 충돌처리 확인 (양방향 체크 - 다형성 콜라이더 지원)
			if (leftCollider->IsCollision(rightCollider) || rightCollider->IsCollision(leftCollider))
			{
				// 이전 프레임 O, 현재 프레임 O
				if (prevCollision[collisionID])
				{
					// 충돌체 중 하나라도 삭제예정인 경우 충돌 해제
					if (leftCollider->IsReservedDelete() || rightCollider->IsReservedDelete())
					{
						leftCollider->OnCollisionExit(rightCollider);
						rightCollider->OnCollisionExit(leftCollider);
						prevCollision[collisionID] = false;
					}
					else
					{
						leftCollider->OnCollisionStay(rightCollider);
						rightCollider->OnCollisionStay(leftCollider);
						prevCollision[collisionID] = true;
					}
				}
				// 이전 프레임 X, 현재 프레임 O
				else
				{
					// 충돌체 중 하나라도 삭제예정인 경우 충돌 진입을 하지 않음
					if (leftCollider->IsReservedDelete() || rightCollider->IsReservedDelete())
					{
						prevCollision[collisionID] = false;
					}
					else
					{
						leftCollider->OnCollisionEnter(rightCollider);
						rightCollider->OnCollisionEnter(leftCollider);
						prevCollision[collisionID] = true;
					}
				}
			}
			else
			{
				// 이전 프레임 O, 현재 프레임 X
				if (prevCollision[collisionID])
				{
					leftCollider->OnCollisionExit(rightCollider);
					rightCollider->OnCollisionExit(leftCollider);
				}
				// 이전 프레임 X, 현재 프레임 X
				else
				{
					// 아무것도 하지 않음
				}
				prevCollision[collisionID] = false;
			}
		}
	}
}

UINT64 CCollisionManager::CollisionID(UINT leftID, UINT rightID)
{
	UINT64 result = 0;
	if (leftID < rightID)
	{
		result |= (UINT64)leftID << 32;
		result |= rightID;
		return result;
	}
	else
	{
		result |= (UINT64)rightID << 32;
		result |= leftID;
		return result;
	}
}

vector<HitResult> CCollisionManager::BoxTrace(const Vec2& center, const Vec2& halfSize, UINT targetLayer, bool bDrawDebug)
{
	vector<HitResult> results;

	for (CCollider* collider : colliderList[targetLayer])
	{
		if (collider->IsReservedDelete())
			continue;

		// AABB 충돌 체크
		Vec2 colPos = collider->GetPos();
		Vec2 colHalf = collider->GetScale() * 0.5f;

		if (abs(center.x - colPos.x) < halfSize.x + colHalf.x &&
			abs(center.y - colPos.y) < halfSize.y + colHalf.y)
		{
			HitResult result;
			result.collider = collider;

			// 각 AABB의 Min/Max 계산
			Vec2 minA = center - halfSize;
			Vec2 maxA = center + halfSize;
			Vec2 minB = colPos - colHalf;
			Vec2 maxB = colPos + colHalf;

			// 겹친 영역(Intersection)의 Min/Max 도출
			float interMinX = max(minA.x, minB.x);
			float interMinY = max(minA.y, minB.y);
			float interMaxX = min(maxA.x, maxB.x);
			float interMaxY = min(maxA.y, maxB.y);

			// 중심점 계산
			result.hitCenter = Vec2((interMinX + interMaxX) * 0.5f, (interMinY + interMaxY) * 0.5f);
			
			results.push_back(result);
		}
	}
	
	if (bDrawDebug)
	{
		bool bHit = !results.empty();
		COLORREF color = bHit ?    RGB(0, 255, 0) : RGB(255, 0, 0);
		DrawDebugBox(center,halfSize, color, 0.1f );
	}
	
	return results;
}

void CCollisionManager::DrawDebugBox(const Vec2& center, const Vec2& halfSize, COLORREF color, float duration)
{
	debugBoxes.push_back({ center, halfSize, color, duration });
}

void CCollisionManager::RenderDebug()
{
	float dt = DT;

	for (auto it = debugBoxes.begin(); it != debugBoxes.end();)
	{
		Vec2 screenPos = CAMERA->WorldToScreenPoint(it->center);
		RENDER->SetPen(PenType::Solid, it->color);
		RENDER->SetBrush(BrushType::Null);
		RENDER->Rect(
			screenPos.x - it->halfSize.x,
			screenPos.y - it->halfSize.y,
			screenPos.x + it->halfSize.x,
			screenPos.y + it->halfSize.y);
		RENDER->SetPen();
		RENDER->SetBrush();

		it->duration -= dt;
		if (it->duration <= 0)
			it = debugBoxes.erase(it);
		else
			++it;
	}
}
