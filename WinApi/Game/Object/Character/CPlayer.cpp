#include "pch.h"
#include "CPlayer.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Ability/Player/Ability_AirAttack.h"
#include "Game/Ability/Player/Ability_ComboAttack.h"
#include "Game/Ability/Player/Ability_Crouch.h"
#include "Game/Ability/Player/Ability_CrouchAttack.h"
#include "Game/Ability/Player/Ability_Jump.h"
#include "Game/Ability/Player/Ability_Parry.h"
#include "Game/Ability/Player/Ability_Slide.h"
#include "Game/Ability/Player/Ability_UseFlask.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CMapManager.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CPlayer::CPlayer() : currentHP(0), maxHP(0), currentMP(0), maxMP(0), currentFlask(0), maxFlask(0), prevVelocity(0, 0), bWasMovingInput(false), ladderX(0), ladderTopY(0), ladderBottomY(0)
{
	name = TEXT("플레이어");
	jumpForce = JUMP_FORCE;
	isPersistent = true;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	CCharacter::Init();
	
	// Rigidbody
	rigidbody = new CRigidbody();
	rigidbody->SetGravityScale(1.6f);
	AddChild(rigidbody);

	// Collider
	colScale = Vec2(42, 72);
	colOffset = Vec2(0, -36);
	collider->SetScale(colScale);
	collider->SetOffset(colOffset);
	collider->SetLayer(ELayer::Player);
	
	// Abilities
	AddAbility<Ability_ComboAttack>(EAbility::Attack);
	AddAbility<Ability_AirAttack>(EAbility::AirAttack);
	AddAbility<Ability_Slide>(EAbility::Slide);
	AddAbility<Ability_Parry>(EAbility::Parry);
	AddAbility<Ability_Crouch>(EAbility::Crouch);
	AddAbility<Ability_CrouchAttack>(EAbility::CrouchAttack);
	AddAbility<Ability_UseFlask>(EAbility::UseFlask);
	AddAbility<Ability_Jump>(EAbility::Jump);
	
	// Animations
	AddAnimation(AnimKey::Idle, TEXT("Animations/Penitent/penitent_idle_anim.json"), true);
	AddAnimation(AnimKey::Run, TEXT("Animations/Penitent/penitent_running_anim.json"), true);
	AddAnimation(AnimKey::JumpStart_Inplace, TEXT("Animations/Penitent/jump_start_inplace.json"), false);
	AddAnimation(AnimKey::JumpStart_Moving, TEXT("Animations/Penitent/jump_start_moving.json"), false);
	AddAnimation(AnimKey::Fall_Inplace, TEXT("Animations/Penitent/penitent_falling_loop.json"), true);
	AddAnimation(AnimKey::Fall_Moving, TEXT("Animations/Penitent/falling_moving.json"), true);
	AddAnimation(AnimKey::Landed_Inplace, TEXT("Animations/Penitent/jump_landed_inplace.json"), false);
	AddAnimation(AnimKey::Landed_Moving, TEXT("Animations/Penitent/jump_landed_moving.json"), false);
	AddAnimation(AnimKey::Combo1, TEXT("Animations/Penitent/penitent_attack_combo_1.json"), false);
	AddAnimation(AnimKey::Combo2, TEXT("Animations/Penitent/penitent_attack_combo_2.json"), false);
	AddAnimation(AnimKey::Combo3, TEXT("Animations/Penitent/penitent_attack_combo_3.json"), false);
	AddAnimation(AnimKey::AirCombo1, TEXT("Animations/Penitent/penitent_jumping_attack1.json"), false);
	AddAnimation(AnimKey::AirCombo2, TEXT("Animations/Penitent/penitent_jumping_attack2.json"), false);
	AddAnimation(AnimKey::Slide, TEXT("Animations/Penitent/penitent_dodge_anim.json"), false);
	AddAnimation(AnimKey::Parry, TEXT("Animations/Penitent/penitent_parry.json"), false);
	AddAnimation(AnimKey::ParrySuccess, TEXT("Animations/Penitent/penitent_parry_success.json"), false);
	AddAnimation(AnimKey::ParryCounter, TEXT("Animations/Penitent/penitent_parry_counter.json"), false);
	AddAnimation(AnimKey::Crouch, TEXT("Animations/Penitent/penitent_crouch_anim.json"), false);
	AddAnimation(AnimKey::CrouchUp, TEXT("Animations/Penitent/penitent_crouch_up_anim.json"), false);
	AddAnimation(AnimKey::CrouchAttack, TEXT("Animations/Penitent/penitent_crouch_attack_anim.json"), false);
	AddAnimation(AnimKey::UseFlask,TEXT("Animations/Penitent/penitent_healthposion_anim.json"), false);
	AddAnimation(AnimKey::Climbing,TEXT("Animations/Penitent/penitent_ladder_climb_loop_anim.json"), true);
	
	// 초기 스탯값 적용
	InitStartupStats();
}

void CPlayer::OnEnable()
{
	CCharacter::OnEnable();

	// 상태 초기화 (씬 전환 시)
	bWasMovingInput = false;
	prevVelocity = Vec2(0, 0);

	if (stateSystem)
	{
		stateSystem->RemoveTagAll(Tag_Moving);
	}
	if (rigidbody)
	{
		rigidbody->SetVelocity(Vec2(0, 0));
		rigidbody->SetGrounded(false);
	}
	if (abilitySystem)
	{
		abilitySystem->CancelAbilitiesWithTag(Tag_AbilityAnimation);
		abilitySystem->CancelAbilitiesWithTag(Tag_Moving);
	}
	

	animator->Play(AnimKey::Idle, false);
}

void CPlayer::Update()
{
	CCharacter::Update();
	HandleCombatInput();
	HandleActionInput();
	UpdateMovement();
	UpdateStates();
	CheckVelocityChanged();
	UpdateAnimation();
}

void CPlayer::HandleCombatInput()
{
	if (INPUT->ButtonDown('K'))
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

	if (INPUT->ButtonDown('J'))
	{
		abilitySystem->TryActivateAbility(EAbility::Parry);
	}
}

void CPlayer::HandleActionInput()
{
	// 사다리 타기 중일 때
	if (stateSystem->HasTag(Tag_Climbing))
	{
		// 점프로 사다리 이탈
		if (INPUT->ButtonDown(VK_SPACE))
		{
			stateSystem->RemoveTag(Tag_Climbing);
			rigidbody->UseGravity(true);
			abilitySystem->TryActivateAbility(EAbility::Jump);
		}
		return;
	}

	// 사다리 진입 (W/S 키로 사다리 영역에서 시작)
	if (stateSystem->HasTag(Tag_CanClimb) && !stateSystem->HasTag(Tag_Climbing))
	{
		// 사다리 상단/하단 근처인지 체크 (여유값 10픽셀)
		constexpr float LADDER_EDGE_THRESHOLD = 10.f;
		float footY = collider->GetPos().y + collider->GetScale().y * 0.5f;
		bool bAtLadderTop = footY < ladderTopY + LADDER_EDGE_THRESHOLD;
		bool bAtLadderBottom = footY > ladderBottomY - LADDER_EDGE_THRESHOLD;

		// 상단 근처면 S키로만, 하단 근처면 W키로만 진입 가능
		bool canEnterWithW = INPUT->ButtonDown('W') && !bAtLadderTop;
		bool canEnterWithS = INPUT->ButtonDown('S') && !bAtLadderBottom;

		if (canEnterWithW || canEnterWithS)
		{
			// S키로 진입 시: 플랫폼 통과 허용 + 캐릭터 중심을 사다리 상단에 맞춤
			if (canEnterWithS)
			{
				bShouldIgnorePlatform = true;
				SetIsGrounded(false);
				SetPos(Vec2(GetPos().x, ladderTopY));
			}

			abilitySystem->CancelAbilitiesWithTag(Tag_Moving);
			stateSystem->AddTagUnique(Tag_Climbing);
			rigidbody->UseGravity(false);
			rigidbody->SetVelocity(Vec2(0.f, 0.f));
			return;
		}
	}

	// Slide
	if (INPUT->ButtonDown(VK_SHIFT))
	{
		abilitySystem->TryActivateAbility(EAbility::Slide);
	}

	// Use Flask
	if (INPUT->ButtonDown('F'))
	{
		abilitySystem->TryActivateAbility(EAbility::UseFlask);
	}

	// Crouch
	if (INPUT->ButtonDown('S'))
	{
		abilitySystem->TryActivateAbility(EAbility::Crouch);
	}
	if (INPUT->ButtonUp('S'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Crouch_Released);
	}

	// JUMP
	if (INPUT->ButtonDown(VK_SPACE))
	{
		abilitySystem->TryActivateAbility(EAbility::Jump);
		abilitySystem->TriggerEvent(EGameEvent::Input_Jump_Pressed); // 점프 시도 이후 trigger해야함.
	}
}

void CPlayer::UpdateMovement()
{
	// 정지
	if (stateSystem->HasTag(Tag_StopVelocity))
	{
		rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
		return;
	}

	if (stateSystem->HasTag(Tag_BlockMovement))
	{
		return;
	}

	Vec2 velocity = rigidbody->GetVelocity();

	// 사다리 타기 중
	if (stateSystem->HasTag(Tag_Climbing))
	{
		velocity.x = 0.f;
		velocity.y = 0.f;
		SetPos(Vec2(ladderX, pos.y));

		// 현재 위치 계산
		float centerY = collider->GetPos().y;
		float footY = centerY + collider->GetScale().y * 0.5f;
		float midY = (centerY + footY) * 0.5f;  // (중심 + 발) / 2

		// 상하 이동 (경계 체크)
		if (INPUT->ButtonStay('W'))
		{
			velocity.y = -CLIMB_SPEED;
			
			// midY가 상단을 벗어나면
			if (midY <= ladderTopY)
			{
				// 사다리 탈출 -> 플랫폼 위에 착지
				// 발이 ladderTopY에 오도록 위치 스냅
				float colHalfY = collider->GetScale().y * 0.5f;
				float offsetY = collider->GetOffset().y;
				SetPos(Vec2(ladderX, ladderTopY - colHalfY - offsetY));

				stateSystem->RemoveTagAll(Tag_Climbing);
				rigidbody->UseGravity(true);
				rigidbody->SetVelocity(Vec2(0.f, 0.f));
				return;
			}
		}
		else if (INPUT->ButtonStay('S') && footY < ladderBottomY)
		{
			velocity.y = CLIMB_SPEED;
			ignoredPlatformID = GetCurrentGroundID();
		}

		rigidbody->SetVelocity(velocity);
		animator->SetReverse(velocity.y > 0.f); // 하강시 역재생
		return;
	}

	// 일반 이동
	bool bIsMovingInput = INPUT->ButtonStay('A') || INPUT->ButtonStay('D');

	if (INPUT->ButtonStay('A'))
	{
		velocity.x = -MOVE_SPEED;
		SetForward(-1);
	}
	else if (INPUT->ButtonStay('D'))
	{
		velocity.x = MOVE_SPEED;
		SetForward(1);
	}
	else if (bIsGrounded)
	{
		// 지면에서만 마찰 적용
		constexpr float FRICTION = 2000.f;
		if (velocity.x > 0)
			velocity.x = max(0.f, velocity.x - FRICTION * DT);
		else if (velocity.x < 0)
			velocity.x = min(0.f, velocity.x + FRICTION * DT);
	}

	// 입력 상태 변화 시 태그 업데이트
	if (bIsMovingInput && !bWasMovingInput)
	{
		stateSystem->AddTag(Tag_Moving);
	}
	else if (!bIsMovingInput && bWasMovingInput)
	{
		stateSystem->RemoveTag(Tag_Moving);
	}
	bWasMovingInput = bIsMovingInput;

	rigidbody->SetVelocity(velocity);
}

void CPlayer::UpdateAnimation()
{
	animator->SetDirection(GetForward());

	// Ability가 애니메이션을 제어 중이면 스킵
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 사다리 타기 중
	if (stateSystem->HasTag(Tag_Climbing))
	{
		if (INPUT->ButtonStay('W') || INPUT->ButtonStay('S'))
		{
			animator->Play(AnimKey::Climbing, false, nullptr, [this]()
			{
				animator->SetReverse(false);
			});	
		}
		else
		{
			animator->Stop();
		}
		return;
	}

	// locomotion
	if (stateSystem->HasTag(Tag_Airborne))
	{
		if (IsNearlyEqual(rigidbody->GetVelocity().x, 0))
		{
			animator->Play(AnimKey::Fall_Inplace, false);
		}
		else
		{
			animator->Play(AnimKey::Fall_Moving, false);
		}
	}
	else if (stateSystem->HasTag(Tag_Moving) && !stateSystem->HasTag(Tag_BlockMovement))
	{
		animator->Play(AnimKey::Run, false);
	}
	else if (stateSystem->HasTag(Tag_Crouching))
	{
		animator->Play(AnimKey::Crouch,false);
	}
	else
	{
		animator->Play(AnimKey::Idle, false);
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
	CCharacter::OnCollisionEnter(other);
}

void CPlayer::OnDamage(CGameObject* source, const CombatContext& context)
{
	abilitySystem->TriggerEvent(EGameEvent::Hit,source);
	
	// Spawn VFX
	Vec2 spawnPos = context.hitResult.hitCenter;
	int spawnDirection = source->GetForward();
	
	if (context.value > 0.0001f)
	{
		// // Spawn Hit VFX
		if (CVFX* vfx = VFX->CreateVFX(GetPlayerHitVfxKey(context.damageType), spawnPos, spawnDirection))
		{
			vfx->PlayVFX();	
		}
		
		// Spawn Blood VFX
		if (CVFX* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
		{
			vfx->PlayVFX();
		}
		
		// Apply damage
		float newHP = currentHP - context.value;
		SetCurrentHP(newHP);
	}
}

void CPlayer::InitStartupStats()
{
	SetMaxHP(MAX_HP);
	SetCurrentHP(MAX_HP);
	SetMaxMP(MAX_MP);
	SetCurrentMP(MAX_MP);
	SetMaxFlask(MAX_FLASK);
	SetCurrentFlask(MAX_FLASK);
}

Vec2 CPlayer::GetKnockbackVelocity(CGameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER);
}

wstring CPlayer::GetPlayerHitVfxKey(EDamageType damageType)
{
	return VFXKey::PlayerHit;
}

void CPlayer::SetCurrentHP(float value)
{
	value = min(value,maxHP);
	UpdateHP(currentHP, value);
}

void CPlayer::SetMaxHP(float value)
{
	UpdateHP(maxHP, value);
}

void CPlayer::SetCurrentMP(float value)
{
	value = min(value, maxMP);
	UpdateMP(currentMP, value);
}

void CPlayer::SetMaxMP(float value)
{
	UpdateMP(maxMP, value);
}

void CPlayer::SetCurrentFlask(int value)
{
	int oldValue = currentFlask;
	currentFlask = max(value, 0);
	currentFlask = min(currentFlask, maxFlask);
	
	if (oldValue != currentFlask)
	{
		GAMEUI->SetPlayerFlask(currentFlask,maxFlask);
	}
	
	if (currentFlask > 0)
	{
		stateSystem->AddTagUnique(Tag_FlaskRemaining);
	}
	else
	{
		stateSystem->RemoveTag(Tag_FlaskRemaining);
	}
}

void CPlayer::SetMaxFlask(int value)
{
	int oldValue = maxFlask;
	maxFlask = max(0,value);
	
	if (oldValue != maxFlask)
	{
		GAMEUI->SetPlayerFlask(currentFlask,maxFlask);
	}
}

void CPlayer::UpdateHP(float& attribute, float value) const
{
	value = max(value, 0.0f);
	
	float oldValue = attribute;
	attribute = value;
	
	if (!IsNearlyEqual(attribute, oldValue))
	{
		GAMEUI->SetPlayerHP(currentHP,maxHP);
	}
}

void CPlayer::UpdateMP(float& attribute, float value) const
{
	value = max(value, 0.0f);
	
	float oldValue = attribute;
	attribute = value;
	
	if (!IsNearlyEqual(attribute, oldValue))
	{
		GAMEUI->SetPlayerHP(currentMP,maxMP);
	}
}

void CPlayer::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);
	
	// 앉기
	if (TagAdded(oldTags, newTags, Tag_Crouching) || TagAdded(oldTags, newTags, Tag_Sliding))
	{
		Vec2 crouchScale = colScale * Vec2(1.0f, 0.5f);
		Vec2 crouchOffset = colOffset + crouchScale * Vec2(0.0f, 0.5f);
		collider->SetScale(crouchScale);
		collider->SetOffset(crouchOffset);
	}
	// 앉기 해제
	else if (TagRemoved(oldTags, newTags, Tag_Crouching) || TagRemoved(oldTags, newTags, Tag_Sliding))
	{
		collider->SetScale(colScale);
		collider->SetOffset(colOffset);
		
	}
	
	// 착지
	if (TagAdded(oldTags, newTags, Tag_Grounded))
	{
		// 착지 시 점프 공격 소진 태그 리셋
		stateSystem->RemoveTag(Tag_AirAttackExhausted);
		// 착지 시 사다리 타기 해제
		stateSystem->RemoveTag(Tag_Climbing);
	}

	// 사다리 타기 해제 시
	if (TagRemoved(oldTags, newTags, Tag_Climbing))
	{
		rigidbody->UseGravity(true);
		bShouldIgnorePlatform = false;
	}
}

void CPlayer::CheckVelocityChanged()
{
	Vec2 curVelocity = rigidbody->GetVelocity();

	// x 또는 y 부호가 바뀌었는지 체크
	bool xSignChanged = (prevVelocity.x * curVelocity.x) < 0;
	bool ySignChanged = (prevVelocity.y * curVelocity.y) < 0;

	if (xSignChanged || ySignChanged)
	{
		abilitySystem->TriggerEvent(EGameEvent::VelocityChanged);
	}

	prevVelocity = curVelocity;
}
