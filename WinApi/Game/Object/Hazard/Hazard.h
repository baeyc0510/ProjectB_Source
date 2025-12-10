#pragma once

class Animator;
class BoxCollider;

class Hazard : public GameObject
{
public:
	Hazard();
	virtual ~Hazard();

	/*~ Hazard Interface ~*/
	void SetDamage(float dmg) { damage = dmg; }
	void SetDamageInterval(float interval) { damageInterval = interval; }

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

	/*~ Hazard Interface ~*/
	virtual void OnSpawn();
	void DealDamageToPlayer(GameObject* player);
	
	void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);

protected:
	Animator* animator = nullptr;
	BoxCollider* collider = nullptr;

	float damage = 10.f;
	float damageInterval = 1.f;
	float lastDamageTime = 0.f;
	bool bIsDestroyed = false;
};
