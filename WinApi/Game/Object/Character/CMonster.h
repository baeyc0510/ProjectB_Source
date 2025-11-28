#pragma once
#include "CCharacter.h"
#include "Game/Interface/CombatInterface.h"

class CRigidbody;

class CMonster : public CCharacter, public ICombatInterface
{
public:
	CMonster();
	~CMonster() override;
	
protected:
	/*~ CGameObject Interfaces ~*/
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void Render() override;
	void OnDisable() override;
	void Release() override; 
	
	void OnCollisionEnter(CCollider* other) override;
	void OnCollisionStay(CCollider* other) override;
	void OnCollisionExit(CCollider* other) override;
	
	/*~ ICombatInterface ~*/
	void OnDamage(CGameObject* source, const CombatContext& context) override;

	/*~ CMonster Interface ~*/
	virtual void UpdateState();
	Vec2 GetKnockbackVelocity(CGameObject* source, const CombatContext& context);
	
	
private:
	// 컴포넌트
	CRigidbody* rigidbody;
	CCollider* collider;

	bool bIsGrounded;
	
	//temp
	float timer = 0.0f;
	float interval = 1.5f;
};

