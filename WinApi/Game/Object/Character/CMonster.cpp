#include "pch.h"
#include "CMonster.h"

#include "Game/AnimKey.h"
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
	collider->SetLayer(ELayer::Monster);
	AddChild(collider);
	
	// Abilities
	AddAbility<Ability_MonsterAttack>(EAbility::Attack);
	AddAbility<Ability_HitReaction>(EAbility::Hit);
	AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);
	
	// Animations
	AddAnimation(AnimKey::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true);
	AddAnimation(AnimKey::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false);
	AddAnimation(AnimKey::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false);
	AddAnimation(AnimKey::ParryHit, TEXT("Animations/Enemy/acolyte_parry_reaction_anim.json"), false);
	AddAnimation(AnimKey::Walking, TEXT("Animations/Enemy/acolite_walking_anim.json"), true);
}

void CMonster::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(AnimKey::Idle);
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

	// UpdateMetaCollision();  // 콜라이더 기반 충돌로 대체됨 (OnCollisionStay)
	UpdateStates();
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

	animator->Play(AnimKey::Idle);
}

void CMonster::OnDamage(CGameObject* source, const CombatContext& context)
{
	// DEBUG
	Logger::Debug(name + TEXT(" Hit!"));
	
	// Trigger Event
	abilitySystem->TriggerEvent(EGameEvent::Hit,source);
	
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
		abilitySystem->CancelAbilitiesWithTag(Tag_Hit);
		abilitySystem->TryActivateAbility(EAbility::Hit);
	}
}

Vec2 CMonster::GetKnockbackVelocity(CGameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER * -0.6f);
}