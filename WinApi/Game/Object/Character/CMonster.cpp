#include "pch.h"
#include "CMonster.h"

#include "Game/Enum.h"
#include "Game/Ability/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_MonsterAttack.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

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
	
	// Abilities
	AddAbility<Ability_MonsterAttack>(EAbility::Attack);
	AddAbility<Ability_HitReaction>(EAbility::Hit);
	
	// Animations
	AddAnimation(TEXT("Idle"),TEXT("Animations/Enemy/acolite_idle_anim.json"),true);
	AddAnimation(TEXT("Attack"),TEXT("Animations/Enemy/acolyte_attack_anim.json"),false);
	AddAnimation(TEXT("Hit"),TEXT("Animations/Enemy/acolyte_get_hit_anim.json"),false);
}

void CMonster::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(TEXT("Idle"));
}

void CMonster::Update()
{
	CCharacter::Update();
	
	timer += DT;
	if (timer >= interval)
	{
		abilitySystem->TryActivateAbility(EAbility::Attack);
		timer = 0.0f;
	}
	
	UpdateState();
	animator->SetDirection(GetForward());
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

void CMonster::UpdateState()
{
	// 착지 체크
	if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
	{
		stateSystem->RemoveTag(Tag_Airborne);
		stateSystem->AddTag(Tag_Grounded);
		abilitySystem->TriggerEvent(EGameEvent::Landed);
	}
	if (!bIsGrounded)
	{
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTag(Tag_Airborne);
	}
	
	// 정지
	if (stateSystem->HasTag(Tag_StopVelocity)) 
	{
		rigidbody->SetVelocity(Vec2(0.0f,0.0f));
		return;
	}
	
	if (stateSystem->HasTag(Tag_AbilityAnimation))
	{
		return;
	}
	
	animator->Play(TEXT("Idle"));
}

void CMonster::OnDamage(CGameObject* source, const CombatContext& context)
{
	// DEBUG
	Logger::Debug(name + TEXT(" Hit!"));
	
	// Trigger Event
	abilitySystem->TriggerEvent(EGameEvent::Hit);
	
	// Spawn VFX
	Vec2 spawnPos = context.hitResult.hitCenter;
	int spawnDirection = source->GetForward();
	
	// Spawn Hit VFX
	if (!context.vfxKey.empty())
	{
		if (CVFX* vfx = VFX->CreateVFX(context.vfxKey, spawnPos, spawnDirection))
		{
			vfx->PlayVFX();	
		}
	}
	
	if (context.value > 0.0001f)
	{
		// Spawn Blood VFX
		if (CVFX* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
		{
			vfx->PlayVFX();
		}
		
		// Hit Reaction
		abilitySystem->CancelAbility(EAbility::Hit);
		if (abilitySystem->TryActivateAbility(EAbility::Hit))
		{
			// Knockback
			Vec2 velocity = GetKnockbackVelocity(source, context);
			rigidbody->SetVelocity(velocity);
			SetForward(velocity.x * -1.0f); // 날아가는 방향 반대를 바라보게 하기
		}
	}
}

Vec2 CMonster::GetKnockbackVelocity(CGameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	float power = 1.0f;
	return Vec2(power * dirX, power * -0.6f);
}