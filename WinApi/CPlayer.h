#pragma once
#include "CGameObject.h"
#include "Enum.h"

class CRigidbody;
class CAnimator;
class CCollider;

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

	CCollider* GetCollider() { return m_pCollider; }

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
	CRigidbody* m_pRigidbody;
	CCollider*  m_pCollider;
	float		speed;
	float		m_fJumpForce;

	PlayerState m_eState;
	bool		m_bIsGrounded;
	Vec2		moveDir;
	Vec2		lookDir;
	bool		isMove;
	int			m_iDirection;
};