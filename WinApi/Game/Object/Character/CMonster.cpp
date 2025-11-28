#include "pch.h"
#include "CMonster.h"

#include "Game/AnimKeys.h"
#include "Game/Enum.h"
#include "Game/Ability/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_MonsterAttack.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CMonster::CMonster()
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
	AddAnimation(Anim::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true);
	AddAnimation(Anim::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false);
	AddAnimation(Anim::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false);
}

void CMonster::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(Anim::Idle);
}

void CMonster::Update()
{
	CCharacter::Update();

	attackTimer += DT;
	if (attackTimer >= ATTACK_INTERVAL)
	{
		abilitySystem->TryActivateAbility(EAbility::Attack);
		attackTimer = 0.0f;
	}

	UpdateGroundState();
	UpdateAnimation();
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

void CMonster::UpdateAnimation()
{
	if (stateSystem->HasTag(Tag_StopVelocity))
	{
		rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
		return;
	}

	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	animator->Play(Anim::Idle);
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
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER * -0.6f);
}