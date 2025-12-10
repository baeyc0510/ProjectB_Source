#include "pch.h"
#include "CharacterMovement.h"

#include "Rigidbody.h"
#include "Game/Enum.h"
#include "Game/Object/Character/Character.h"
#include "Components/LineCollider.h"

namespace
{
	// 경사면 접촉 판정 범위
	constexpr float MIN_SLOPE_CONTACT = -5.f;      // 경사면 위로 떠있는 허용 범위
	constexpr float MAX_SLOPE_PENETRATION = 50.f;  // 경사면에 파묻힌 최대 허용 깊이

	// 가파른 경사면 미끄러짐 속도
	constexpr float STEEP_SLOPE_SLIDE_SPEED = 300.f;

	// 끼임 상태에서 밀려나는 속도
	constexpr float SQUASH_PUSH_SPEED = 150.f;

	// 바닥 스냅 오프셋 (미세한 떨림 방지)
	constexpr float GROUND_SNAP_OFFSET = 1.f;

	// 단차 허용 높이 (벽으로 처리하지 않고 넘어갈 수 있는 높이)
	constexpr float STEP_HEIGHT_TOLERANCE = 2.5f;

	// 엣지 감지용 상수
	constexpr float EDGE_CHECK_DISTANCE = 5.0f;
	constexpr float EDGE_CHECK_SIZE = 5.0f;
	constexpr float EDGE_SAFETY_MARGIN = 5.0f;
}

CharacterMovement::CharacterMovement()
{
}

CharacterMovement::~CharacterMovement()
{
}

void CharacterMovement::SetConfig(const FMovementConfig& inConfig)
{
	config = inConfig;
	maxSlopeAngleRad = config.maxSlopeAngle * 3.14159265f / 180.0f;
}

void CharacterMovement::ComponentInit()
{
	// 컴포넌트 캐시
	rigidbody = owner->GetComponent<Rigidbody>();
	collider = owner->GetComponent<BoxCollider>();

	// 필수 컴포넌트 검증
	assert(rigidbody && "CharacterMovement requires Rigidbody");
	assert(collider && "CharacterMovement requires BoxCollider");

	// 기본 설정 적용
	maxSlopeAngleRad = config.maxSlopeAngle * 3.14159265f / 180.0f;
}

void CharacterMovement::ComponentOnEnable()
{
	Component<GameObject>::ComponentOnEnable();

	// 상태 초기화
	groundState.Reset();
	frameFlags.Reset();
	ignoredPlatformID = 0;
}

void CharacterMovement::ComponentLateUpdate()
{
	// 프레임별 충돌 정보 리셋
	frameFlags.Reset();
}

void CharacterMovement::ResetGroundState()
{
	groundState.activeGroundID = 0;
	groundState.activeGroundTop = -FLT_MAX;
	groundState.groundMinX = -FLT_MAX;
	groundState.groundMaxX = FLT_MAX;
}

void CharacterMovement::SetGrounded(bool value)
{
	groundState.bIsGrounded = value;
	rigidbody->SetGrounded(groundState.bIsGrounded);
}

void CharacterMovement::HandleCollisionEnter(Collider* other)
{
	HandleCollisionStay(other);
}

void CharacterMovement::HandleCollisionStay(Collider* other)
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
	LineCollider* lineCollider = dynamic_cast<LineCollider*>(other);
	if (lineCollider)
	{
		HandleLineGround(lineCollider, isPlatform);
	}
	else
	{
		HandleBoxGround(other, isPlatform);
	}
}

void CharacterMovement::HandleLineGround(LineCollider* lineCollider, bool isPlatform)
{
	Vec2 characterColPos = collider->GetPos();
	Vec2 characterColHalf = collider->GetScale() * 0.5f;
	Vec2 velocity = rigidbody->GetVelocity();

	// X 범위 체크
	if (!lineCollider->IsInXRange(characterColPos.x))
		return;

	// 접촉 깊이 계산
	float footY = characterColPos.y + characterColHalf.y;
	float slopeY = lineCollider->GetYAt(characterColPos.x);
	float penetration = footY - slopeY;

	// 원웨이 플랫폼: 상승 중이면 통과
	if (isPlatform && velocity.y < 0)
		return;

	// 경사면 접촉 범위를 벗어나면 무시
	if (penetration <= MIN_SLOPE_CONTACT || penetration >= MAX_SLOPE_PENETRATION)
		return;

	// activeGround 갱신 (더 아래 슬로프 우선)
	if (groundState.activeGroundID == 0 || slopeY > groundState.activeGroundTop)
	{
		groundState.activeGroundID = lineCollider->GetID();
		groundState.activeGroundTop = slopeY;

		Vec2 start = lineCollider->GetWorldStart();
		Vec2 end = lineCollider->GetWorldEnd();
		groundState.groundMinX = min(start.x, end.x);
		groundState.groundMaxX = max(start.x, end.x);
	}

	if (lineCollider->GetID() != groundState.activeGroundID)
		return;

	SetGrounded(true);

	// 슬로프 방향 및 각도 계산
	Vec2 slopeStart = lineCollider->GetWorldStart();
	Vec2 slopeEnd = lineCollider->GetWorldEnd();
	bool bIsUpRight = slopeEnd.y < slopeStart.y;
	float slopeAngle = lineCollider->GetSlopeAngle();
	bool bIsGoingUp = (velocity.x > 0 && bIsUpRight) || (velocity.x < 0 && !bIsUpRight);

	// 경사면 타입에 따라 처리
	if (slopeAngle > maxSlopeAngleRad)
	{
		HandleSteepSlope(lineCollider, velocity, bIsUpRight, slopeAngle);
	}
	else
	{
		HandleGentleSlope(lineCollider, velocity, bIsGoingUp, slopeAngle);
	}
}

void CharacterMovement::HandleBoxGround(Collider* other, bool isPlatform)
{
	Vec2 characterColPos = collider->GetPos();
	Vec2 characterColHalf = collider->GetScale() * 0.5f;
	Vec2 velocity = rigidbody->GetVelocity();

	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;
	float otherTop = otherPos.y - otherHalf.y;
	float otherBottom = otherPos.y + otherHalf.y;

	float myHead = characterColPos.y - characterColHalf.y;
	float myFoot = characterColPos.y + characterColHalf.y;

	// activeGround 갱신 (더 아래 Ground 우선)
	if (groundState.activeGroundID == 0 || otherTop > groundState.activeGroundTop)
	{
		groundState.activeGroundID = other->GetID();
		groundState.activeGroundTop = otherTop;
		groundState.groundMinX = otherPos.x - otherHalf.x;
		groundState.groundMaxX = otherPos.x + otherHalf.x;
	}

	// 원웨이 플랫폼: 상승 중이거나 이미 통과한 경우 무시
	if (isPlatform)
	{
		if (velocity.y < 0 || myFoot > otherBottom)
			return;
	}

	// AABB 겹침 계산
	float overlapX = (characterColHalf.x + otherHalf.x) - abs(characterColPos.x - otherPos.x);
	float overlapY = (characterColHalf.y + otherHalf.y) - abs(characterColPos.y - otherPos.y);

	if (overlapX <= 0 || overlapY <= 0)
		return;

	// 충돌 방향 결정: 수직 vs 수평
	if (overlapY <= overlapX)
	{
		// 끼임 상태 (발이 장애물 바닥보다 아래)
		if (myFoot > otherBottom)
		{
			Character* character = dynamic_cast<Character*>(owner);
			int pushDir = character ? character->GetForward() : 1;
			HandleSquashState(pushDir, overlapX);
			return;
		}

		// 바닥 충돌
		if (myFoot >= otherTop)
		{
			HandleFloorCollision(other, otherPos, otherHalf, overlapY);
		}
		// 천장 충돌
		else if (myHead <= otherBottom)
		{
			HandleCeilingCollision(overlapY);
		}
	}
	// 벽 충돌
	else
	{
		HandleWallCollision(other, overlapX);
	}
}

void CharacterMovement::HandleCollisionExit(Collider* other)
{
	ELayer layer = static_cast<ELayer>(other->GetLayer());

	if (layer != ELayer::Ground && layer != ELayer::Platform)
		return;

	// activeGround exit 시 착지 해제
	if (other->GetID() == groundState.activeGroundID)
	{
		SetGrounded(false);
		ResetGroundState();
	}

	// 경사면 exit 시 미끄러짐 해제
	if (layer == ELayer::Ground && dynamic_cast<LineCollider*>(other))
	{
		groundState.bIsOnSteepSlope = false;
	}

	// 엣지 감지 (AI용)
	if (config.bBlockAtEdges)
	{
		HandleGroundExit(other);
	}
}

void CharacterMovement::HandleGroundExit(Collider* other)
{
	// 현재 이동 방향 (velocity 기반)
	Vec2 velocity = rigidbody->GetVelocity();
	int dir = (velocity.x >= 0) ? 1 : -1;

	// 앞에 ground가 있는지 확인
	if (CheckGroundAhead(dir))
		return;

	// 엣지 도달 플래그 설정
	frameFlags.bReachedEdge = true;
	frameFlags.edgeDirection = dir;

	// 즉시 정지
	rigidbody->SetVelocity(Vec2(0.0f, velocity.y));

	// 안전 위치로 보정
	float edgeMinX, edgeMaxX;
	LineCollider* lineCollider = dynamic_cast<LineCollider*>(other);
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

	if (dir > 0)
	{
		float safeX = edgeMaxX - myHalfWidth - EDGE_SAFETY_MARGIN;
		safePos.x = safeX - offset.x;
	}
	else
	{
		float safeX = edgeMinX + myHalfWidth + EDGE_SAFETY_MARGIN;
		safePos.x = safeX - offset.x;
	}
	owner->SetPos(safePos);
}

bool CharacterMovement::CheckGroundAhead(int direction)
{
	Vec2 traceCenter = owner->GetPos();
	traceCenter.x += direction * EDGE_CHECK_DISTANCE;
	Vec2 traceHalfSize = { EDGE_CHECK_SIZE, EDGE_CHECK_SIZE };

	auto groundResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Ground);
	auto platformResults = COLLISION->BoxTrace(traceCenter, traceHalfSize, (UINT)ELayer::Platform);

	return !groundResults.empty() || !platformResults.empty();
}

void CharacterMovement::AddMoveInput(float direction)
{
	moveInput = direction;
}

void CharacterMovement::ProcessMovement()
{
	Vec2 vel = rigidbody->GetVelocity();

	// 입력이 있으면 속도 적용
	if (moveInput != 0.f)
	{
		vel.x = moveInput * moveSpeed;
	}
	// 입력이 없고 지면에 있으면 마찰 적용
	else if (groundState.bIsGrounded)
	{
		if (vel.x > 0)
			vel.x = max(0.f, vel.x - friction * DT);
		else if (vel.x < 0)
			vel.x = min(0.f, vel.x + friction * DT);
	}

	rigidbody->SetVelocity(vel);

	// 입력 리셋 (매 프레임 호출되어야 함)
	moveInput = 0.f;
}

/*~ HandleLineGround Helpers ~*/

void CharacterMovement::HandleSteepSlope(LineCollider* lineCollider, const Vec2& velocity, bool bIsUpRight, float slopeAngle)
{
	Vec2 characterColHalf = collider->GetScale() * 0.5f;

	groundState.bIsOnSteepSlope = true;

	Vec2 newPos = owner->GetPos();
	float snapY = lineCollider->GetYAt(newPos.x + collider->GetOffset().x);
	newPos.y = snapY - characterColHalf.y - collider->GetOffset().y;
	owner->SetPos(newPos);

	float slideDir = bIsUpRight ? -1.f : 1.f;
	Vec2 slideVel;
	slideVel.x = slideDir * STEEP_SLOPE_SLIDE_SPEED * cosf(slopeAngle);
	slideVel.y = STEEP_SLOPE_SLIDE_SPEED * sinf(slopeAngle);
	rigidbody->SetVelocity(slideVel);
}

void CharacterMovement::HandleGentleSlope(LineCollider* lineCollider, Vec2& velocity, bool bIsGoingUp, float slopeAngle)
{
	Vec2 characterColHalf = collider->GetScale() * 0.5f;

	groundState.bIsOnSteepSlope = false;

	if (velocity.y < 0)
		return;

	// 경사면 오르막 속도 보정: 수평 이동 속도를 경사면 방향에 맞게 감속
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

/*~ HandleBoxGround Helpers ~*/

void CharacterMovement::HandleSquashState(int pushDir, float overlapX)
{
	frameFlags.bBeingSquashed = true;
	frameFlags.squashPushDirection = pushDir;

	float pushAmount = SQUASH_PUSH_SPEED * DT;
	pushAmount = min(pushAmount, overlapX);

	Vec2 newPos = owner->GetPos();
	newPos.x += pushDir * pushAmount;
	owner->SetPos(newPos);
}

void CharacterMovement::HandleFloorCollision(Collider* other, const Vec2& otherPos, const Vec2& otherHalf, float overlapY)
{
	Vec2 velocity = rigidbody->GetVelocity();
	float otherTop = otherPos.y - otherHalf.y;

	if (velocity.y < 0)
		return;

	// activeGround 갱신
	if (groundState.activeGroundID == 0 || otherTop > groundState.activeGroundTop)
	{
		groundState.activeGroundID = other->GetID();
		groundState.activeGroundTop = otherTop;
		groundState.groundMinX = otherPos.x - otherHalf.x;
		groundState.groundMaxX = otherPos.x + otherHalf.x;
	}

	if (other->GetID() != groundState.activeGroundID)
		return;

	SetGrounded(true);

	Vec2 newPos = owner->GetPos();
	newPos.y = otherTop + GROUND_SNAP_OFFSET;
	owner->SetPos(newPos);

	if (velocity.y > 0)
	{
		velocity.y = 0.f;
		rigidbody->SetVelocity(velocity);
	}
}

void CharacterMovement::HandleCeilingCollision(float overlapY)
{
	Vec2 velocity = rigidbody->GetVelocity();

	if (velocity.y >= 0)
		return;

	velocity.y = 0.f;
	rigidbody->SetVelocity(velocity);

	Vec2 newPos = owner->GetPos();
	newPos.y += overlapY;
	owner->SetPos(newPos);
}

void CharacterMovement::HandleWallCollision(Collider* other, float overlapX)
{
	Vec2 characterColPos = collider->GetPos();
	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;
	float otherTop = otherPos.y - otherHalf.y;
	float myFoot = characterColPos.y + collider->GetScale().y * 0.5f;

	// 단차가 아주 작으면 넘어감
	if (abs(otherTop - myFoot) < STEP_HEIGHT_TOLERANCE)
		return;

	Vec2 velocity = rigidbody->GetVelocity();
	float pushDir = (characterColPos.x < otherPos.x) ? -1.f : 1.f;  // 캐릭터 밀려날 방향
	int wallDir = (pushDir < 0) ? 1 : -1;  // 벽이 있는 방향 (pushDir의 반대)

	bool movingIntoWall = (pushDir < 0 && velocity.x > 0) || (pushDir > 0 && velocity.x < 0);
	if (movingIntoWall)
	{
		velocity.x = 0.f;
		rigidbody->SetVelocity(velocity);

		frameFlags.bHitWall = true;
		frameFlags.wallHitDirection = wallDir;
	}

	Vec2 newPos = owner->GetPos();
	newPos.x += pushDir * overlapX;
	owner->SetPos(newPos);
}
