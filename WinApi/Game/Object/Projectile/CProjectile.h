#pragma once
#include "Game/Interface/CombatInterface.h"

class CAnimator;
class CRigidbody;
class CBoxCollider;

class CProjectile : public CGameObject, public ICombatInterface
{
public:
	CProjectile();
	virtual ~CProjectile();

	/*~ CProjectile Interface ~*/
	void SetProjectileOwner(CGameObject* inOwner) { projectileOwner = inOwner; }
	void SetVelocity(Vec2 vel);
	void SetDamage(float dmg) { damage = dmg; }
	void SetLifetime(float time) { lifetime = time; bHasLifetime = true; }

	void Launch(Vec2 velocity);

protected:
	/*~ CGameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	void OnCollisionEnter(CCollider* other) override;

	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

	/*~ CProjectile Interface (Virtual) ~*/
	virtual void OnHitGround(Vec2 hitPos);
	virtual void OnHitWall(Vec2 hitPos);
	virtual void OnHitPlayer(CGameObject* player);
	virtual void OnDestroyed();

	void DestroySelf();

protected:
	CGameObject* projectileOwner = nullptr;
	CAnimator* animator = nullptr;
	CRigidbody* rigidbody = nullptr;
	CBoxCollider* collider = nullptr;

	float damage = 10.f;
	float lifetime = 0.f;
	float remainingLifetime = 0.f;
	bool bHasLifetime = false;
	bool bIsDestroyed = false;
};
