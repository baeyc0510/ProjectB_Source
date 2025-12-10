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
	void DealDamageToPlayer(CGameObject* player);
	
	void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);

protected:
	CAnimator* animator = nullptr;
	CBoxCollider* collider = nullptr;

	float damage = 10.f;
	float damageInterval = 1.f;
	float lastDamageTime = 0.f;
	bool bIsDestroyed = false;
};
