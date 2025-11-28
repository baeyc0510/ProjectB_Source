#include "pch.h"
#include "CPlayer.h"

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
	: rigidbody(nullptr)
	, collider(nullptr)
	, speed(300.f)
	, jumpForce(500.f)
	, bIsGrounded(false)
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
	collider->SetScale(Vec2(42, 72));
	collider->SetOffset(Vec2(0,-36));
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
	AddAnimation(TEXT("Idle"), TEXT("Animations/Penitent/penitent_idle_anim.json"),true);
	AddAnimation(TEXT("Run"), TEXT("Animations/Penitent/penitent_running_anim.json"),true);
	AddAnimation(TEXT("Jump"), TEXT("Animations/Penitent/penitent_jump_anim.json"),false);
	AddAnimation(TEXT("Fall"), TEXT("Animations/Penitent/penitent_falling_loop.json"),true);
	AddAnimation(TEXT("Combo1"), TEXT("Animations/Penitent/penitent_attack_combo_1.json"),false);
	AddAnimation(TEXT("Combo2"), TEXT("Animations/Penitent/penitent_attack_combo_2.json"),false);
	AddAnimation(TEXT("Combo3"), TEXT("Animations/Penitent/penitent_attack_combo_3.json"),false);
	AddAnimation(TEXT("AirCombo1"), TEXT("Animations/Penitent/penitent_jumping_attack1.json"),false);
	AddAnimation(TEXT("AirCombo2"), TEXT("Animations/Penitent/penitent_jumping_attack2.json"),false);
	AddAnimation(TEXT("Slide"), TEXT("Animations/Penitent/penitent_dodge_anim.json"),false);
	AddAnimation(TEXT("Parry"), TEXT("Animations/Penitent/penitent_parry.json"),false);
	AddAnimation(TEXT("ParrySuccess"), TEXT("Animations/Penitent/penitent_parry_success.json"),false);
	AddAnimation(TEXT("ParryCounter"), TEXT("Animations/Penitent/penitent_parry_counter.json"),false);
	AddAnimation(TEXT("Crouch"), TEXT("Animations/Penitent/penitent_crouch_anim.json"),false);
	AddAnimation(TEXT("CrouchUp"), TEXT("Animations/Penitent/penitent_crouch_up_anim.json"),false);
	AddAnimation(TEXT("CrouchAttack"), TEXT("Animations/Penitent/penitent_crouch_attack_anim.json"),false);
}

void CPlayer::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(L"Idle", false);
}

void CPlayer::Update()
{
	CCharacter::Update();
	HandleInput();
	UpdateState();
	animator->SetDirection(GetForward());
}

void CPlayer::HandleInput()
{
	// 기본 공격
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
	// 패리 (가드)
	if (INPUT->ButtonDown('D'))
	{
		abilitySystem->TryActivateAbility(EAbility::Parry);
	}
	// 슬라이드
	if (INPUT->ButtonDown(VK_SHIFT))
	{
		abilitySystem->TryActivateAbility(EAbility::Slide);
	}
	// 웅크리기
	if (INPUT->ButtonDown(VK_DOWN))
	{
		abilitySystem->TryActivateAbility(EAbility::Crouch);
	}
	if (INPUT->ButtonUp(VK_DOWN))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Crouch_Released);
	}
	
	
	// 이동 불가
	if (stateSystem->HasTag(Tag_BlockMovement))
	{
		return;
	}
	
	// 이동
	Vec2 velocity = rigidbody->GetVelocity();
	
	// 이동 입력
	if (INPUT->ButtonStay(VK_LEFT))
	{
		velocity.x = -speed;
		SetForward(-1);
		stateSystem->AddTagUnique(Tag_Moving);
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		velocity.x = speed;
		SetForward(1);
		stateSystem->AddTagUnique(Tag_Moving);
	}
	else
	{
		velocity.x = 0;
		stateSystem->RemoveTag(Tag_Moving);
	}

	// 점프 입력 TODO: 어빌리티로 이동
	if (INPUT->ButtonDown(VK_SPACE) && stateSystem->HasTag(Tag_Grounded))
	{
		velocity.y = -jumpForce;
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTagUnique(Tag_Airborne);
	}

	rigidbody->SetVelocity(velocity);
}

void CPlayer::UpdateState()
{
	// 착지 체크
	if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
	{
		stateSystem->RemoveTag(Tag_Airborne);
		stateSystem->AddTagUnique(Tag_Grounded);
		abilitySystem->TriggerEvent(EGameEvent::Landed);
	}
	if (!bIsGrounded)
	{
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTagUnique(Tag_Airborne);
	}
	
	// 정지
	if (stateSystem->HasTag(Tag_StopVelocity)) 
	{
		rigidbody->SetVelocity(Vec2(0.0f,0.0f));
		return;
	}

	// Ability가 애니메이션을 제어 중이면 기본 애니메이션 로직 스킵
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 애니메이션 결정
	if (stateSystem->HasTag(Tag_Airborne))
	{
		animator->Play(L"Fall", false);
	}
	else if (stateSystem->HasTag(Tag_Moving))
	{
		animator->Play(L"Run", false);
	}
	else
	{
		animator->Play(L"Idle", false);
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

void CPlayer::OnCollisionEnter(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		// 땅에 처음 닿는 순간 위치 보정
		Vec2 velocity = rigidbody->GetVelocity();
		if (velocity.y > 0) // 아래로 떨어지고 있을 때만
		{
			float playerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
			float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

			float overlap = playerBottom - groundTop;
			if (overlap > 0)
			{
				Vec2 playerPos = GetPos();
				playerPos.y -= overlap;
				SetPos(playerPos);
			}
		}
	}
}

void CPlayer::OnCollisionStay(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		bIsGrounded = true;

		// 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
		float playerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
		float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

		float overlap = playerBottom - groundTop;
		if (overlap > 0)
		{
			Vec2 playerPos = GetPos();
			playerPos.y -= overlap;
			SetPos(playerPos);
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

void CPlayer::OnCollisionExit(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		bIsGrounded = false;
	}
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
	float power = 100.0f;
	return Vec2(power * dirX, power * 1.0f);
}

wstring CPlayer::GetPlayerHitVfxKey(EDamageType damageType)
{
	return TEXT("VFX_Attack1");
}
