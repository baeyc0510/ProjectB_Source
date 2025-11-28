#include "pch.h"
#include "CPlayer.h"

#include "Game/AnimKeys.h"
#include "Game/Ability/Player/Ability_AirAttack.h"
#include "Game/Ability/Player/Ability_ComboAttack.h"
#include "Game/Ability/Player/Ability_Crouch.h"
#include "Game/Ability/Player/Ability_CrouchAttack.h"
#include "Game/Ability/Player/Ability_Parry.h"
#include "Game/Ability/Player/Ability_Slide.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CPlayer::CPlayer()
{
	name = TEXT("플레이어");
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	CCharacter::Init();
	
	// Rigidbody
	rigidbody = new CRigidbody();
	AddChild(rigidbody);

	// Collider
	collider = new CCollider();
	standingColScale = Vec2(42, 72);
	standingColOffset = Vec2(0, -36);
	collider->SetScale(standingColScale);
	collider->SetOffset(standingColOffset);
	collider->SetLayer(Layer::Player);
	AddChild(collider);
	
	// Abilities
	AddAbility<Ability_ComboAttack>(EAbility::Attack);
	AddAbility<Ability_AirAttack>(EAbility::AirAttack);
	AddAbility<Ability_Slide>(EAbility::Slide);
	AddAbility<Ability_Parry>(EAbility::Parry);
	AddAbility<Ability_Crouch>(EAbility::Crouch);
	AddAbility<Ability_CrouchAttack>(EAbility::CrouchAttack);
	
	// Animations
	AddAnimation(Anim::Idle, TEXT("Animations/Penitent/penitent_idle_anim.json"), true);
	AddAnimation(Anim::Run, TEXT("Animations/Penitent/penitent_running_anim.json"), true);
	AddAnimation(Anim::Jump, TEXT("Animations/Penitent/penitent_jump_anim.json"), false);
	AddAnimation(Anim::Fall, TEXT("Animations/Penitent/penitent_falling_loop.json"), true);
	AddAnimation(Anim::Combo1, TEXT("Animations/Penitent/penitent_attack_combo_1.json"), false);
	AddAnimation(Anim::Combo2, TEXT("Animations/Penitent/penitent_attack_combo_2.json"), false);
	AddAnimation(Anim::Combo3, TEXT("Animations/Penitent/penitent_attack_combo_3.json"), false);
	AddAnimation(Anim::AirCombo1, TEXT("Animations/Penitent/penitent_jumping_attack1.json"), false);
	AddAnimation(Anim::AirCombo2, TEXT("Animations/Penitent/penitent_jumping_attack2.json"), false);
	AddAnimation(Anim::Slide, TEXT("Animations/Penitent/penitent_dodge_anim.json"), false);
	AddAnimation(Anim::Parry, TEXT("Animations/Penitent/penitent_parry.json"), false);
	AddAnimation(Anim::ParrySuccess, TEXT("Animations/Penitent/penitent_parry_success.json"), false);
	AddAnimation(Anim::ParryCounter, TEXT("Animations/Penitent/penitent_parry_counter.json"), false);
	AddAnimation(Anim::Crouch, TEXT("Animations/Penitent/penitent_crouch_anim.json"), false);
	AddAnimation(Anim::CrouchUp, TEXT("Animations/Penitent/penitent_crouch_up_anim.json"), false);
	AddAnimation(Anim::CrouchAttack, TEXT("Animations/Penitent/penitent_crouch_attack_anim.json"), false);
}

void CPlayer::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(Anim::Idle, false);

	// StateSystem 이벤트 구독
	stateSystem->OnStateChanged.Add([this](StateTag oldTags, StateTag newTags) {
		OnStateChanged(oldTags, newTags);
	});
}

void CPlayer::Update()
{
	CCharacter::Update();
	HandleCombatInput();
	HandleActionInput();
	HandleMovementInput();
	UpdateGroundState();
	UpdateAnimation();
	animator->SetDirection(GetForward());
}

void CPlayer::HandleCombatInput()
{
	if (INPUT->ButtonDown('A'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Attack_Pressed);

		if (stateSystem->HasTag(Tag_Crouching))
		{
			abilitySystem->TryActivateAbility(EAbility::CrouchAttack);
		}
		else if (stateSystem->HasTag(Tag_Airborne))
		{
			abilitySystem->TryActivateAbility(EAbility::AirAttack);
		}
		else if (stateSystem->HasTag(Tag_Grounded))
		{
			abilitySystem->TryActivateAbility(EAbility::Attack);
		}
	}

	if (INPUT->ButtonDown('D'))
	{
		abilitySystem->TryActivateAbility(EAbility::Parry);
	}
}

void CPlayer::HandleActionInput()
{
	if (INPUT->ButtonDown(VK_SHIFT))
	{
		abilitySystem->TryActivateAbility(EAbility::Slide);
	}

	if (INPUT->ButtonDown(VK_DOWN))
	{
		abilitySystem->TryActivateAbility(EAbility::Crouch);
	}
	if (INPUT->ButtonUp(VK_DOWN))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Crouch_Released);
	}
}

void CPlayer::HandleMovementInput()
{
	if (stateSystem->HasTag(Tag_BlockMovement))
	{
		return;
	}

	Vec2 velocity = rigidbody->GetVelocity();

	if (INPUT->ButtonStay(VK_LEFT))
	{
		velocity.x = -MOVE_SPEED;
		SetForward(-1);
		stateSystem->AddTagUnique(Tag_Moving);
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		velocity.x = MOVE_SPEED;
		SetForward(1);
		stateSystem->AddTagUnique(Tag_Moving);
	}
	else
	{
		velocity.x = 0;
		stateSystem->RemoveTag(Tag_Moving);
	}

	// TODO: 점프 어빌리티로 이동
	if (INPUT->ButtonDown(VK_SPACE) && stateSystem->HasTag(Tag_Grounded))
	{
		velocity.y = -JUMP_FORCE;
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTagUnique(Tag_Airborne);
	}

	rigidbody->SetVelocity(velocity);
}

void CPlayer::UpdateAnimation()
{
	// 정지
	if (stateSystem->HasTag(Tag_StopVelocity))
	{
		rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
		return;
	}

	// Ability가 애니메이션을 제어 중이면 기본 애니메이션 로직 스킵
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 애니메이션 결정
	if (stateSystem->HasTag(Tag_Airborne))
	{
		animator->Play(Anim::Fall, false);
	}
	else if (stateSystem->HasTag(Tag_Moving))
	{
		animator->Play(Anim::Run, false);
	}
	else
	{
		animator->Play(Anim::Idle, false);
	}
}

void CPlayer::Render()
{
	CCharacter::Render();
}

void CPlayer::OnDisable()
{
	CCharacter::OnDisable();
}

void CPlayer::Release()
{
	CCharacter::Release();
}


void CPlayer::OnDamage(CGameObject* source, const CombatContext& context)
{
	abilitySystem->TriggerEvent(EGameEvent::Hit);
	
	// Spawn VFX
	Vec2 spawnPos = context.hitResult.hitCenter;
	int spawnDirection = source->GetForward();
	
	// // Spawn Hit VFX
	// if (CVFX* vfx = VFX->CreateVFX(GetPlayerHitVfxKey(context.damageType), spawnPos, spawnDirection))
	// {
	// 	vfx->PlayVFX();	
	// }
	
	// Spawn Blood VFX
	if (context.value > 0.0001f)
	{
		if (CVFX* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
		{
			vfx->PlayVFX();
		}
	}
}

Vec2 CPlayer::GetKnockbackVelocity(CGameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER);
}

wstring CPlayer::GetPlayerHitVfxKey(EDamageType damageType)
{
	return TEXT("VFX_Attack1");
}

void CPlayer::OnStateChanged(StateTag oldTags, StateTag newTags)
{
	// 앉기
	if (TagAdded(oldTags, newTags, Tag_Crouching))
	{
		Vec2 crouchScale = standingColScale * Vec2(1.0f, 0.5f);
		Vec2 crouchOffset = standingColOffset + crouchScale * Vec2(0.0f, 0.5f);
		collider->SetScale(crouchScale);
		collider->SetOffset(crouchOffset);
	}
	// 앉기 해제
	else if (TagRemoved(oldTags, newTags, Tag_Crouching))
	{
		collider->SetScale(standingColScale);
		collider->SetOffset(standingColOffset);
	}

	// 착지 시 점프 공격 소진 태그 리셋
	if (TagAdded(oldTags, newTags, Tag_Grounded))
	{
		stateSystem->RemoveTag(Tag_AirAttackExhausted);
	}
}
