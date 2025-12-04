#include "pch.h"
#include "CCharacterMovement.h"

#include "CRigidbody.h"
#include "Game/Enum.h"
#include "Game/Object/Character/CCharacter.h"
#include "Components/CLineCollider.h"

CCharacterMovement::CCharacterMovement()
{
}

CCharacterMovement::~CCharacterMovement()
{
}

void CCharacterMovement::SetConfig(const FMovementConfig& inConfig)
{
	config = inConfig;
	maxSlopeAngleRad = config.maxSlopeAngle * 3.14159265f / 180.0f;
}

void CCharacterMovement::ComponentInit()
{
	// 컴포넌트 캐시
	rigidbody = owner->GetComponent<CRigidbody>();
	collider = owner->GetComponent<CBoxCollider>();

	// 기본 설정 적용
	maxSlopeAngleRad = config.maxSlopeAngle * 3.14159265f / 180.0f;
}

void CCharacterMovement::ComponentOnEnable()
{
	Component::ComponentOnEnable();

	// 상태 초기화
	bIsGrounded = false;
	bIsOnSteepSlope = false;
	bWasOnSteepSlope = false;
	activeGroundID = 0;
	activeGroundTop = -FLT_MAX;
	ignoredPlatformID = 0;
	platformMinX = -FLT_MAX;
	platformMaxX = FLT_MAX;

	ResetFrameState();
}

void CCharacterMovement::LateUpdate()
{
	// 이전 프레임 상태 갱신
	bWasOnSteepSlope = bIsOnSteepSlope;

	// 프레임별 충돌 정보 리셋
	ResetFrameState();
}

void CCharacterMovement::ResetFrameState()
{
	bHitWall = false;
	wallHitDirection = 0;
	bReachedEdge = false;
	edgeDirection = 0;
}

void CCharacterMovement::ResetGroundState()
{
	activeGroundID = 0;
	activeGroundTop = -FLT_MAX;
	platformMinX = -FLT_MAX;
	platformMaxX = FLT_MAX;
}

void CCharacterMovement::SetGrounded(bool value)
{
	bIsGrounded = value;

	if (bIsGrounded)
	{
		ignoredPlatformID = 0;
	}
	else
	{
		ResetGroundState();
	}

	if (rigidbody)
	{
		rigidbody->SetGrounded(bIsGrounded);
	}
}

void CCharacterMovement::SetIgnorePlatform(UINT platformID)
{
	ignoredPlatformID = platformID;
}

void CCharacterMovement::HandleCollisionEnter(CCollider* other)
{
	HandleCollisionStay(other);
}

void CCharacterMovement::HandleCollisionStay(CCollider* other)
{
	ELayer layer = static_cast<ELayer>(other->GetLayer());
	bool isGround = (layer == ELayer::Ground);
	bool isPlatform = (layer == ELayer::Platform);

	if (!isGround && !isPlatform)
		return;

	if (isPlatform && other->GetID() == ignoredPlatformID)
		return;

	if (!collider || !rigidbody)
		return;

	// 라인 콜라이더(경사면) 또는 박스 콜라이더 처리
	CLineCollider* lineCollider = dynamic_cast<CLineCollider*>(other);
	if (lineCollider)
	{
		HandleLineGround(lineCollider, isPlatform);
	}
	else
	{
		HandleBoxGround(other, isPlatform);
	}
}

void CCharacterMovement::HandleLineGround(CLineCollider* lineCollider, bool isPlatform)
{
	Vec2 characterColPos = collider->GetPos();
	Vec2 characterColHalf = collider->GetScale() * 0.5f;
	Vec2 velocity = rigidbody->GetVelocity();

	float footY = characterColPos.y + characterColHalf.y;

	if (!lineCollider->IsInXRange(characterColPos.x))
		return;

	float slopeY = lineCollider->GetYAt(characterColPos.x);
	float penetration = footY - slopeY;

	// 원웨이 플랫폼: 상승 중이면 통과
	if (isPlatform && velocity.y < 0)
		return;

	// 슬로프 위에 있거나 약간 파고들었을 때
	if (penetration <= -5.f || penetration >= 50.f)
		return;

	// 더 아래(Y가 큰) 슬로프를 activeGround로 선택
	if (activeGroundID == 0 || slopeY > activeGroundTop)
	{
		activeGroundID = lineCollider->GetID();
		activeGroundTop = slopeY;

		Vec2 start = lineCollider->GetWorldStart();
		Vec2 end = lineCollider->GetWorldEnd();
		platformMinX = min(start.x, end.x);
		platformMaxX = max(start.x, end.x);
	}

	if (lineCollider->GetID() != activeGroundID)
		return;

	// 착지 상태
	SetGrounded(true);

	// 슬로프 방향 판별
	Vec2 slopeStart = lineCollider->GetWorldStart();
	Vec2 slopeEnd = lineCollider->GetWorldEnd();
	bool bIsUpRight = slopeEnd.y < slopeStart.y;

	float slopeAngle = lineCollider->GetSlopeAngle();
	bool bIsGoingUp = (velocity.x > 0 && bIsUpRight) || (velocity.x < 0 && !bIsUpRight);

	// 가파른 경사면
	if (slopeAngle > maxSlopeAngleRad)
	{
		bIsOnSteepSlope = true;

		Vec2 newPos = owner->GetPos();
		float snapY = lineCollider->GetYAt(newPos.x + collider->GetOffset().x);
		newPos.y = snapY - characterColHalf.y - collider->GetOffset().y;
		owner->SetPos(newPos);

		constexpr float SLIDE_SPEED = 300.f;
		float slideDir = bIsUpRight ? -1.f : 1.f;
		velocity.x = slideDir * SLIDE_SPEED * cosf(slopeAngle);
		velocity.y = SLIDE_SPEED * sinf(slopeAngle);
		rigidbody->SetVelocity(velocity);
		return;
	}

	// 완만한 경사면
	bIsOnSteepSlope = false;

	if (velocity.y < 0)
		return;

	// 경사면 속도 보정
	Vec2 newCharacterPos = owner->GetPos();
	if (bIsGoingUp && abs(velocity.x) > 0.1f)
	{
		float cosAngle = cosf(slopeAngle);
		float velocityAdjust = velocity.x * (1.f - cosAngle) * DT;
		newCharacterPos.x -= velocityAdjust;
	}

	float adjustedSlopeY = lineCollider->GetYAt(newCharacterPos.x + collider->GetOffset().x);
	newCharacterPos.y = adjustedSlopeY - characterColHalf.y - collider->GetOffset().y;
	owner->SetPos(newCharacterPos);

	if (velocity.y > 0)
	{
		velocity.y = 0.f;
		rigidbody->SetVelocity(velocity);
	}
}

void CCharacterMovement::HandleBoxGround(CCollider* other, bool isPlatform)
{
	Vec2 characterColPos = collider->GetPos();
	Vec2 characterColHalf = collider->GetScale() * 0.5f;
	Vec2 velocity = rigidbody->GetVelocity();

	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;
	float otherTop = otherPos.y - otherHalf.y;
	float otherBottom = otherPos.y + otherHalf.y;

	Vec2 myPos = collider->GetPos();
	Vec2 myHalf = collider->GetScale() * 0.5f;
	float myHead = myPos.y - myHalf.y;
	float myFoot = myPos.y + myHalf.y;

	// 더 아래 Ground를 activeGround로 선택
	if (activeGroundID == 0 || otherTop > activeGroundTop)
	{
		activeGroundID = other->GetID();
		activeGroundTop = otherTop;
		platformMinX = otherPos.x - otherHalf.x;
		platformMaxX = otherPos.x + otherHalf.x;
	}

	// 원웨이 플랫폼
	if (isPlatform)
	{
		if (velocity.y < 0)
			return;

		if (myFoot > otherBottom)
			return;
	}

	// AABB 겹침 계산
	float overlapX = (characterColHalf.x + otherHalf.x) - abs(characterColPos.x - otherPos.x);
	float overlapY = (characterColHalf.y + otherHalf.y) - abs(characterColPos.y - otherPos.y);

	if (overlapX <= 0 || overlapY <= 0)
		return;

	// 수직 충돌 (바닥 또는 천장)
	if (overlapY <= overlapX)
	{
		// 바닥 충돌
		if (myFoot >= otherTop)
		{
			if (velocity.y >= 0)
			{
				if (activeGroundID == 0 || otherTop > activeGroundTop)
				{
					activeGroundID = other->GetID();
					activeGroundTop = otherTop;
					platformMinX = otherPos.x - otherHalf.x;
					platformMaxX = otherPos.x + otherHalf.x;
				}

				if (other->GetID() != activeGroundID)
					return;

				SetGrounded(true);

				Vec2 newPos = owner->GetPos();
				newPos.y = otherTop + 1.f;
				owner->SetPos(newPos);

				if (velocity.y > 0)
				{
					velocity.y = 0.f;
					rigidbody->SetVelocity(velocity);
				}
			}
		}
		// 천장 충돌
		else if (myHead <= otherBottom)
		{
			if (velocity.y < 0)
			{
				velocity.y = 0.f;
				rigidbody->SetVelocity(velocity);

				Vec2 newPos = owner->GetPos();
				newPos.y += overlapY;
				owner->SetPos(newPos);
			}
		}
	}
	// 수평 충돌 (벽)
	else
	{
		// 단차가 아주 작으면 넘어감
		if (abs(otherTop - myFoot) < 2.5f)
			return;

		float pushDir = (characterColPos.x < otherPos.x) ? -1.f : 1.f;
		int wallDir = (pushDir < 0) ? 1 : -1;

		bool movingIntoWall = (pushDir < 0 && velocity.x > 0) || (pushDir > 0 && velocity.x < 0);
		if (movingIntoWall)
		{
			velocity.x = 0.f;
			rigidbody->SetVelocity(velocity);

			// 벽 충돌 플래그 설정
			bHitWall = true;
			wallHitDirection = wallDir;
		}

		Vec2 newPos = owner->GetPos();
		newPos.x += pushDir * overlapX;
		owner->SetPos(newPos);
	}
}

void CCharacterMovement::HandleCollisionExit(CCollider* other)
{
	ELayer layer = static_cast<ELayer>(other->GetLayer());

	if (layer != ELayer::Ground && layer != ELayer::Platform)
		return;

	// activeGround exit 시 착지 해제
	if (other->GetID() == activeGroundID)
	{
		SetGrounded(false);
	}

	// 경사면 exit 시 미끄러짐 해제
	if (layer == ELayer::Ground && dynamic_cast<CLineCollider*>(other))
	{
		bIsOnSteepSlope = false;
	}

	// 엣지 감지 (AI용)
	if (config.bBlockAtEdges)
	{
		HandleGroundExit(other);
	}
}

void CCharacterMovement::HandleGroundExit(CCollider* other)
{
	// 현재 이동 방향 (velocity 기반)
	Vec2 velocity = rigidbody->GetVelocity();
	int dir = (velocity.x >= 0) ? 1 : -1;

	// 앞에 ground가 있는지 확인
	if (CheckGroundAhead(dir))
		return;

	// 엣지 도달 플래그 설정
	bReachedEdge = true;
	edgeDirection = dir;

	// 즉시 정지
	rigidbody->SetVelocity(Vec2(0.0f, velocity.y));

	// 안전 위치로 보정
	float edgeMinX, edgeMaxX;
	CLineCollider* lineCollider = dynamic_cast<CLineCollider*>(other);
	if (lineCollider)
	{
		Vec2 start = lineCollider->GetWorldStart();
		Vec2 end = lineCollider->GetWorldEnd();
		edgeMinX = min(start.x, end.x);
		edgeMaxX = max(start.x, end.x);
	}
	else
	{
		Vec2 otherPos = other->GetPos();
		float otherHalfX = other->GetScale().x * 0.5f;
		edgeMinX = otherPos.x - otherHalfX;
		edgeMaxX = otherPos.x + otherHalfX;
	}

	Vec2 safePos = owner->GetPos();
	float myHalfWidth = collider->GetScale().x * 0.5f;
	Vec2 offset = collider->GetOffset();
	constexpr float EDGE_MARGIN = 5.0f;

	if (dir > 0)
	{
		float safeX = edgeMaxX - myHalfWidth - EDGE_MARGIN;
		safePos.x = safeX - offset.x;
	}
	else
	{
		float safeX = edgeMinX + myHalfWidth + EDGE_MARGIN;
		safePos.x = safeX - offset.x;
	}
	owner->SetPos(safePos);
}

bool CCharacterMovement::CheckGroundAhead(int direction)
{
	constexpr float CHECK_AHEAD = 5.0f;
	constexpr float CHECK_SIZE = 5.0f;

	Vec2 traceCenter = owner->GetPos();
	traceCenter.x += direction * CHECK_AHEAD;
	Vec2 traceHalfSize = { CHECK_SIZE, CHECK_SIZE };

	auto groundResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Ground);
	auto platformResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Platform);

	return !groundResults.empty() || !platformResults.empty();
}
