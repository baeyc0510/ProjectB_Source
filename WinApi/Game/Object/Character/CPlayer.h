#pragma once
#include "Game/Enum.h"

class CRigidbody;
class CAnimator;
class CCollider;
class CStateSystem;
class CAbilitySystem;

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

	CCollider* GetCollider() { return collider; }

private:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override; 
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;

private:
	void HandleInput();
	void UpdateState();

	// 컴포넌트
	CAnimator* animator;
	CRigidbody* rigidbody;
	CCollider* collider;
	CStateSystem* stateSystem;
	CAbilitySystem* abilitySystem;

	// 속성
	float speed;
	float jumpForce;
	int direction;
	bool bIsGrounded;
};