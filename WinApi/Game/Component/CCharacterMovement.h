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

// 캐릭터 이동 및 충돌 처리 컴포넌트
// - 지면 착지, 플랫폼 통과, 경사면 처리 등 물리적 이동 담당
// - Character가 매 프레임 상태를 폴링하여 StateSystem 갱신
class CCharacterMovement : public Component<CCharacter>
{
public:
	CCharacterMovement();
	virtual ~CCharacterMovement();

	// Component interface
	void ComponentInit() override;
	void ComponentOnEnable() override;
	void ComponentUpdate() override {}
	void ComponentRender() override {}
	void ComponentRelease() override {}

	// 설정
	void SetConfig(const FMovementConfig& inConfig);
	const FMovementConfig& GetConfig() const { return config; }

	// 충돌 처리 (Character가 호출)
	void HandleCollisionEnter(CCollider* other);
	void HandleCollisionStay(CCollider* other);
	void HandleCollisionExit(CCollider* other);

	// 상태 조회 (Character가 폴링)
	bool IsGrounded() const { return bIsGrounded; }
	bool IsOnSteepSlope() const { return bIsOnSteepSlope; }
	bool WasOnSteepSlope() const { return bWasOnSteepSlope; }
	UINT GetActiveGroundID() const { return activeGroundID; }
	float GetPlatformMinX() const { return platformMinX; }
	float GetPlatformMaxX() const { return platformMaxX; }
	bool HasPlatformBounds() const { return activeGroundID != 0; }

	// 플랫폼 드롭다운
	void SetIgnorePlatform(UINT platformID) { ignoredPlatformID = platformID; }
	void ClearIgnorePlatform() { ignoredPlatformID = 0; }

	// 프레임 종료 시 호출 (이전 상태 갱신)
	void LateUpdate();

	// 벽/엣지 충돌 정보 (AI용, 폴링)
	bool DidHitWall() const { return bHitWall; }
	int GetWallHitDirection() const { return wallHitDirection; }
	bool DidReachEdge() const { return bReachedEdge; }
	int GetEdgeDirection() const { return edgeDirection; }

private:
	// 충돌 처리 내부 함수
	void HandleGroundCollision(CCollider* other, bool isPlatform);
	void HandleBoxGround(CCollider* other, bool isPlatform);
	void HandleLineGround(CLineCollider* lineCollider, bool isPlatform);
	void HandleWallCollision(CCollider* other);
	void HandleGroundExit(CCollider* other);

	// 상태 설정 (내부용)
	void SetGrounded(bool value);

	// 엣지 감지 (AI용)
	bool CheckGroundAhead(int direction);

	// 상태 리셋
	void ResetFrameState();
	void ResetGroundState();

private:
	// 설정
	FMovementConfig config;
	float maxSlopeAngleRad = 0.0f;

	// 착지 상태
	bool bIsGrounded = false;
	bool bIsOnSteepSlope = false;
	bool bWasOnSteepSlope = false;
	UINT activeGroundID = 0;
	float activeGroundTop = -FLT_MAX;
	UINT ignoredPlatformID = 0;
	float platformMinX = -FLT_MAX;
	float platformMaxX = FLT_MAX;

	// 프레임별 충돌 정보 (매 프레임 리셋)
	bool bHitWall = false;
	int wallHitDirection = 0;
	bool bReachedEdge = false;
	int edgeDirection = 0;

	// 컴포넌트 캐시
	CRigidbody* rigidbody = nullptr;
	CBoxCollider* collider = nullptr;
};
