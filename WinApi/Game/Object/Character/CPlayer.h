#pragma once
#include "Game/Enum.h"

class CRigidbody;
class CAnimator;
class CCollider;

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

	CCollider* GetCollider() { return collider; }

private:
	void	Init()			override;
	void	OnEnable()		override;
	void	Update()		override;
	void	Render()		override;
	void	OnDisable()		override;
	void	Release()		override;

	void	OnCollisionEnter(CCollider* other) override;
	void	OnCollisionStay(CCollider* other) override;
	void	OnCollisionExit(CCollider* other) override;

private:
	void	AnimatorUpdate();

	CAnimator*	animator;
	CRigidbody* rigidbody;
	CCollider*  collider;
	float		speed;
	float		jumpForce;

	PlayerState state;
	bool		bIsGrounded;
	Vec2		moveDir;
	Vec2		lookDir;
	bool		isMove;
	int			direction;
};