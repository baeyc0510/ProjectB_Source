#pragma once
#include "Core/Component.h"

class CCharacter;
class CRigidbody;
class CBoxCollider;
class CCollider;
class CLineCollider;

// 이동 설정
struct FMovementConfig
{
	float maxSlopeAngle = 50.0f;			// 오를 수 있는 최대 경사각 (도)
	bool bCanDropThrough = true;			// 플랫폼 드롭다운 가능 여부 (Player: true)
	bool bBlockAtEdges = false;				// 가장자리에서 정지 (AI: true)
	bool bFlipDirectionAtEdge = false;		// 가장자리에서 방향 전환 (AI 순찰: true)
	bool bFlipDirectionAtWall = false;		// 벽에서 방향 전환 (AI 순찰: true)
};

// 지면 상태 (지속적, 착지 해제 시 리셋)
struct FGroundState
{
	bool bIsGrounded = false;
	bool bIsOnSteepSlope = false;
	UINT activeGroundID = 0;
	float activeGroundTop = -FLT_MAX;
	float groundMinX = -FLT_MAX;
	float groundMaxX = FLT_MAX;

	void Reset()
	{
		bIsGrounded = false;
		bIsOnSteepSlope = false;
		activeGroundID = 0;
		activeGroundTop = -FLT_MAX;
		groundMinX = -FLT_MAX;
		groundMaxX = FLT_MAX;
	}
};

// 프레임 이벤트 플래그 (매 프레임 리셋)
struct FFrameFlags
{
	bool bHitWall = false;
	int wallHitDirection = 0;
	bool bReachedEdge = false;
	int edgeDirection = 0;
	bool bBeingSquashed = false;		// 낮은 천장에 끼인 상태
	int squashPushDirection = 0;		// 밀려나는 방향

	void Reset()
	{
		bHitWall = false;
		wallHitDirection = 0;
		bReachedEdge = false;
		edgeDirection = 0;
		bBeingSquashed = false;
		squashPushDirection = 0;
	}
};

// 캐릭터 이동 및 충돌 처리 컴포넌트
// - 지면 착지, 플랫폼 통과, 경사면 처리 등 물리적 이동 담당
// - Character가 매 프레임 상태를 폴링하여 StateSystem 갱신
class CCharacterMovement : public Component<CGameObject>
{
public:
	CCharacterMovement();
	virtual ~CCharacterMovement();

	/*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentOnEnable() override;
	void ComponentUpdate() override {}
	void ComponentLateUpdate() override;
	void ComponentRender() override {}
	void ComponentRelease() override {}

	/*~ CCharacterMovement Interface ~*/
	// 설정
	void SetConfig(const FMovementConfig& inConfig);
	const FMovementConfig& GetConfig() const { return config; }

	// 충돌 처리 (Character가 호출)
	void HandleCollisionEnter(CCollider* other);
	void HandleCollisionStay(CCollider* other);
	void HandleCollisionExit(CCollider* other);

	// 상태 조회 - 지면
	bool IsGrounded() const { return groundState.bIsGrounded; }
	bool IsOnSteepSlope() const { return groundState.bIsOnSteepSlope; }
	UINT GetActiveGroundID() const { return groundState.activeGroundID; }
	float GetGroundMinX() const { return groundState.groundMinX; }
	float GetGroundMaxX() const { return groundState.groundMaxX; }
	bool HasGroundBounds() const { return groundState.activeGroundID != 0; }

	// 상태 설정
	void SetGrounded(bool value);
	void SetIgnorePlatform(UINT platformID) { ignoredPlatformID = platformID; }
	void ClearIgnorePlatform() { ignoredPlatformID = 0; }

	// 벽/엣지 충돌 정보 (AI용, 폴링)
	bool DidHitWall() const { return frameFlags.bHitWall; }
	int GetWallHitDirection() const { return frameFlags.wallHitDirection; }
	bool DidReachEdge() const { return frameFlags.bReachedEdge; }
	int GetEdgeDirection() const { return frameFlags.edgeDirection; }

	// 끼임 상태 (낮은 천장에서 슬라이드 후 끼인 경우)
	bool IsBeingSquashed() const { return frameFlags.bBeingSquashed; }
	int GetSquashPushDirection() const { return frameFlags.squashPushDirection; }

	// 이동 입력 처리
	void SetMoveSpeed(float speed) { moveSpeed = speed; }
	float GetMoveSpeed() const { return moveSpeed; }
	void SetFriction(float value) { friction = value; }

	void AddMoveInput(float direction);  // -1 (left), 0 (none), +1 (right)
	void ProcessMovement();              // 속도 적용 + 마찰 처리

private:
	// 충돌 처리 내부 함수
	void HandleBoxGround(CCollider* other, bool isPlatform);
	void HandleLineGround(CLineCollider* lineCollider, bool isPlatform);
	void HandleGroundExit(CCollider* other);

	// 엣지 감지 (AI용)
	bool CheckGroundAhead(int direction);

	// 상태 리셋
	void ResetGroundState();

private:
	// 설정
	FMovementConfig config;
	float maxSlopeAngleRad = 0.0f;

	// 상태
	FGroundState groundState;
	FFrameFlags frameFlags;
	UINT ignoredPlatformID = 0;		// 무시할 플랫폼 ID (입력값)

	// 이동 상태
	float moveSpeed = 0.f;
	float friction = 2000.f;
	float moveInput = 0.f;			// 이번 프레임 입력 (-1, 0, +1)

	// 컴포넌트 캐시
	CRigidbody* rigidbody = nullptr;
	CBoxCollider* collider = nullptr;
};
