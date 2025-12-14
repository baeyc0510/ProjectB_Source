#pragma once
#include "Game/Interface/CombatInterface.h"

class Animator;
class Rigidbody;
class BoxCollider;

class Projectile : public GameObject, public ICombatInterface
{
public:
	Projectile();
	virtual ~Projectile();

	/*~ Projectile Interface ~*/
	void SetProjectileOwner(GameObject* inOwner) { projectileOwner = inOwner; }
	void SetVelocity(Vec2 vel);
	void UseGravity(bool bUseGravity);
	void SetGravityScale(float gravityScale);
	void SetDamage(float dmg) { damage = dmg; }
	void Launch(Vec2 velocity);

protected:
	/*~ GameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	void OnCollisionEnter(Collider* other) override;
	void OnCollisionStay(Collider* other) override;

	/*~ ICombatInterface ~*/
	void OnDamage(GameObject* source, const CombatContext& context) override;

	/*~ Projectile Interface ~*/
	virtual void OnHitGround(Vec2 hitPos, Collider* groundCol);
	virtual void OnHitWall(Vec2 hitPos);
	virtual void OnHitPlayer(GameObject* player);

	void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);

protected:
	GameObject* projectileOwner = nullptr;
	Animator* animator = nullptr;
	Rigidbody* rigidbody = nullptr;
	BoxCollider* collider = nullptr;

	float damage = 10.f;
};
