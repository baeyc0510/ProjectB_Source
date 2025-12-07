#pragma once

class CAnimator;
class CBoxCollider;

class CHazard : public CGameObject
{
public:
	CHazard();
	virtual ~CHazard();

	/*~ CHazard Interface ~*/
	void SetDamage(float dmg) { damage = dmg; }
	void SetLifetime(float time) { lifetime = time; bHasLifetime = true; }
	void SetDamageInterval(float interval) { damageInterval = interval; }

protected:
	/*~ CGameObject Interface ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override;

	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other) override;

	/*~ CHazard Interface (Virtual) ~*/
	virtual void OnSpawn();
	virtual void OnExpire();
	void DealDamageToPlayer(CGameObject* player);

	void DestroySelf();

protected:
	CAnimator* animator = nullptr;
	CBoxCollider* collider = nullptr;

	float damage = 10.f;
	float lifetime = 5.f;
	float remainingLifetime = 0.f;
	float damageInterval = 1.f;
	float lastDamageTime = 0.f;
	bool bHasLifetime = true;
	bool bIsDestroyed = false;
};
