#include "pch.h"
#include "CMonster.h"

#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"

CMonster::CMonster() : rigidbody(nullptr), collider(nullptr)
{
	name = TEXT("몬스터");
}

CMonster::~CMonster()
{
}

void CMonster::Init()
{
	CCharacter::Init();
	// Rigidbody
	rigidbody = new CRigidbody();
	AddChild(rigidbody);

	// Collider
	collider = new CCollider();
	collider->SetScale(Vec2(32, 66));
	collider->SetOffset(Vec2(0, -33));
	collider->SetLayer(Layer::Monster);
	AddChild(collider);
	
	AddAnimation(TEXT("Idle"),TEXT("Animations/Enemy/acolite_idle_anim.json"),true);
}

void CMonster::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(TEXT("Idle"));
}

void CMonster::Update()
{
	CCharacter::Update();
	animator->Play(TEXT("Idle"));
}

void CMonster::Render()
{
	CCharacter::Render();
}

void CMonster::OnDisable()
{
	CCharacter::OnDisable();
}

void CMonster::Release()
{
	CCharacter::Release();
}

void CMonster::OnCollisionEnter(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		// 땅에 처음 닿는 순간 위치 보정
		Vec2 velocity = rigidbody->GetVelocity();
		if (velocity.y > 0) // 아래로 떨어지고 있을 때만
		{
			float characterBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
			float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

			float overlap = characterBottom - groundTop;
			if (overlap > 0)
			{
				Vec2 characterPos = GetPos();
				characterPos.y -= overlap;
				SetPos(characterPos);
			}
		}
	}
}

void CMonster::OnCollisionStay(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		bIsGrounded = true;

		// 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
		float characterBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
		float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

		float overlap = characterBottom - groundTop;
		if (overlap > 0)
		{
			Vec2 characterPos = GetPos();
			characterPos.y -= overlap;
			SetPos(characterPos);
		}

		// 땅을 뚫고 올라가는 것을 방지
		Vec2 velocity = rigidbody->GetVelocity();
		if (velocity.y > 0)
		{
			velocity.y = 0.f;
			rigidbody->SetVelocity(velocity);
		}
	}
}

void CMonster::OnCollisionExit(CCollider* other)
{
	CCharacter::OnCollisionExit(other);
}
