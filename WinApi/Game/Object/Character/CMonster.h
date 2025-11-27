#pragma once
#include "CCharacter.h"

class CRigidbody;

class CMonster : public CCharacter
{
public:
	CMonster();
	~CMonster() override;
	
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
	// 컴포넌트
	CRigidbody* rigidbody;
	CCollider* collider;

	bool bIsGrounded;
};

