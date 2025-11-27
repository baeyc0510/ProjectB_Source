#pragma once
#include "CCharacter.h"
#include "Game/Enum.h"

class CRigidbody;
class CAnimator;
class CCollider;
class CStateSystem;
class CAbilitySystem;

class CPlayer : public CCharacter
{
public:
	CPlayer();
	~CPlayer() override;
	
	CCollider* GetCollider() { return collider; }

protected:
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
	
private:
	// 컴포넌트
	CRigidbody* rigidbody;
	CCollider* collider;

	// 속성
	float speed;
	float jumpForce;
	bool bIsGrounded;
};