#include "pch.h"
#include "CPlayer.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/SFXKeys.h"
#include "Game/Ability/Ability_HitReaction.h"
#include "Game/Manager/CSFXManager.h"
#include "Game/Ability/Player/Ability_AirAttack.h"
#include "Game/Ability/Player/Ability_ComboAttack.h"
#include "Game/Ability/Player/Ability_Crouch.h"
#include "Game/Ability/Player/Ability_CrouchAttack.h"
#include "Game/Ability/Player/Ability_Jump.h"
#include "Game/Ability/Player/Ability_Parry.h"
#include "Game/Ability/Player/Ability_Slide.h"
#include "Game/Ability/Player/Ability_UseFlask.h"
#include "Game/Ability/Player/Ability_Climb.h"
#include "Game/Ability/Player/Ability_LedgeClimb.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CCharacterMovement.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Game/Object/World/CLedge.h"

CPlayer::CPlayer() : currentHP(0), maxHP(0), currentMP(0), maxMP(0), currentFlask(0), maxFlask(0), prevVelocity(0, 0), bWasMovingInput(false), ladderX(0), ladderTopY(0), ladderBottomY(0)
{
	name = TEXT("플레이어");
	jumpForce = JUMP_FORCE;
	isPersistent = true;
	pushbackForce = Vec2(300.f,150.f);
}

CPlayer::~CPlayer()
{
	
}

void CPlayer::Init()
{
	CCharacter::Init();
	
	// Rigidbody
	rigidbody->SetGravityScale(PLAYER_GRAVITY_SCALE);

	// Collider
	characterScale = Vec2(42, 66);
	colOffset = Vec2(0, -33);
	collider->SetScale(characterScale);
	collider->SetOffset(colOffset);
	collider->SetLayer(ELayer::Player);

	// Movement (Player 기본 설정: 드롭다운 가능, 엣지 블로킹 없음)
	FMovementConfig moveConfig;
	moveConfig.maxSlopeAngle = 50.0f;
	moveConfig.bCanDropThrough = true;
	moveConfig.bBlockAtEdges = false;
	moveConfig.bFlipDirectionAtEdge = false;
	moveConfig.bFlipDirectionAtWall = false;
	movement->SetConfig(moveConfig);
	movement->SetMoveSpeed(MOVE_SPEED);
	
	// Abilities
	AddAbility<Ability_ComboAttack>(EAbility::Attack);
	AddAbility<Ability_AirAttack>(EAbility::AirAttack);
	AddAbility<Ability_Slide>(EAbility::Slide);
	AddAbility<Ability_Parry>(EAbility::Parry);
	AddAbility<Ability_Crouch>(EAbility::Crouch);
	AddAbility<Ability_CrouchAttack>(EAbility::CrouchAttack);
	AddAbility<Ability_UseFlask>(EAbility::UseFlask);
	AddAbility<Ability_Jump>(EAbility::Jump);
	AddAbility<Ability_Climb>(EAbility::Climb);
	AddAbility<Ability_LedgeClimb>(EAbility::HangOnLedge);
	AddAbility<Ability_PlayerPushback>(EAbility::HitReact);
	
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
	AddAnimation(AnimKey::LedgeHang, TEXT("Animations/Penitent/penitent_hangonledge_anim.json"), false);
	AddAnimation(AnimKey::LedgeClimbOver, TEXT("Animations/Penitent/penitent_climbledge.json"), false);
	AddAnimation(AnimKey::Pushback, TEXT("Animations/Penitent/penitent_pushback_anim.json"), false);
	AddAnimation(AnimKey::Pushback_Land, TEXT("Animations/Penitent/penitent_pushback_land_anim.json"), false);
	AddAnimation(AnimKey::Rising, TEXT("Animations/Penitent/player_rising.json"), false);
	
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
	}
	if (movement)
	{
		movement->SetGrounded(false);
	}
	if (abilitySystem)
	{
		abilitySystem->CancelAbilitiesWithTag(Tag_AbilityAnimation);
		abilitySystem->CancelAbilitiesWithTag(Tag_Moving);
	}
}

void CPlayer::Update()
{
	if (bIsDown)
		return;
	
	CCharacter::Update();
	
	ProcessActiveInput();
	UpdatePlayerStates();
	ProcessPassiveAbilities();
	UpdateAnimation();
}

void CPlayer::LateUpdate()
{
	CheckVelocityChanged();
}

void CPlayer::ProcessActiveInput()
{
	HandleCombatInput();
	HandleActionInput();
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
	// Hold 이벤트 트리거
	if (INPUT->ButtonStay('W'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Up_Hold);
	}
	if (INPUT->ButtonStay('S'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Down_Hold);
	}
	if (INPUT->ButtonUp('W'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Up_Released);
	}
	if (INPUT->ButtonUp('S'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Down_Released);
		abilitySystem->TriggerEvent(EGameEvent::Input_Crouch_Released);
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
	
	// Climb or Crouch
	if (INPUT->ButtonDown('W'))
	{
		abilitySystem->TryActivateAbility(EAbility::Climb);
	}
	if (INPUT->ButtonDown('S'))
	{
		if (abilitySystem->TryActivateAbility(EAbility::Climb))
		{
			return;
		}
		abilitySystem->TryActivateAbility(EAbility::Crouch);
	}

	// Jump
	if (INPUT->ButtonDown(VK_SPACE))
	{
		abilitySystem->TryActivateAbility(EAbility::Jump);
		abilitySystem->TriggerEvent(EGameEvent::Input_Jump_Pressed);
	}
}

void CPlayer::UpdatePlayerStates()
{
	UpdateMovementState();
	UpdateStates();
	UpdateLedgeState();
}

void CPlayer::UpdateMovementState()
{
	// 정지 상태
	if (stateSystem->HasTag(Tag_StopVelocity))
		return;
	
	if (!stateSystem->HasTag(Tag_BlockMovement))
	{
		// 이동 입력 처리
		float moveDir = 0.f;
		if (INPUT->ButtonStay('A'))
		{
			moveDir = -1.f;
			SetForward(-1);
		}
		else if (INPUT->ButtonStay('D'))
		{
			moveDir = 1.f;
			SetForward(1);
		}

		// Movement 컴포넌트에 위임
		movement->AddMoveInput(moveDir);

		// Tag 업데이트
		bool bIsMovingInput = (moveDir != 0.f);
		if (bIsMovingInput && !bWasMovingInput)
		{
			stateSystem->AddTag(Tag_Moving);
		}
		else if (!bIsMovingInput && bWasMovingInput)
		{
			stateSystem->RemoveTag(Tag_Moving);
		}
		bWasMovingInput = bIsMovingInput;
	}
	
	if (!stateSystem->HasTag(Tag_FixedVelocity))
	{
		movement->ProcessMovement();	
	}
}

void CPlayer::UpdateLedgeState()
{
	bool bIsFalling = rigidbody->GetVelocity().y > 0;
	bool bCanMove = !stateSystem->HasTag(Tag_BlockMovement);

	if (!bOverlapWithLedge || !bIsFalling || !bCanMove)
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
		return;
	}

	// 방향 체크: 바라보는 방향과 ledge 방향이 일치해야 함
	if (GetForward() != ledgeDirection)
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
		return;
	}

	Vec2 playerPos = collider->GetPos();
	Vec2 playerHalfScale = collider->GetScale() * 0.5f;
	float playerTop = playerPos.y - playerHalfScale.y;
	float checkY = playerTop + LEDGE_CLIMB_THRESHOLD;

	// 기준점이 ledgeTop보다 낮은 경우 CanClimb
	if (checkY < ledgeTop)
	{
		stateSystem->AddTagUnique(Tag_CanClimbLedge);
	}
	else
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
	}
}

void CPlayer::ProcessPassiveAbilities()
{
	abilitySystem->TryActivateAbility(EAbility::HangOnLedge); // 매달리기
}

void CPlayer::HandleAnimationEvent(EGameEvent event)
{
	CCharacter::HandleAnimationEvent(event);
	if (event == EGameEvent::Footstep)
	{
		OnFootstep();
	}
}

void CPlayer::OnFootstep()
{
	// 달리기 중일 때만 발소리 재생
	if (stateSystem->HasTag(Tag_Moving) && stateSystem->HasTag(Tag_Grounded))
	{
		static int footstepIndex = 0;
		const wchar_t* sounds[] = { SFXKey::PlayerRun1, SFXKey::PlayerRun2 };
		SFX->PlayOnce(sounds[footstepIndex % 2]);
		footstepIndex++;
	}
}

void CPlayer::UpdateAnimation()
{
	animator->SetDirection(GetForward());

	// Ability가 애니메이션을 제어 중이면 스킵
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// Locomotion
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

void CPlayer::OnCollisionStay(CCollider* other)
{
	ELayer layer = static_cast<ELayer>( other->GetLayer());
	if (layer == ELayer::Ledge)
	{
		CheckLedge(other);
	}
	
	CCharacter::OnCollisionStay(other);
}

void CPlayer::OnCollisionExit(CCollider* other)
{
	// 설정한 ledge에서 벗어난 경우 ledge정보 초기화
	ELayer layer = static_cast<ELayer>( other->GetLayer());
	if (layer == ELayer::Ledge && ledgeId == other->GetID())
	{
		ClearLedge();
	}
	
	CCharacter::OnCollisionExit(other);
}

void CPlayer::OnDamage(CGameObject* source, const CombatContext& context)
{
	abilitySystem->TriggerEvent(EGameEvent::Hit,source);
	
	if (context.value <= 0.0001f)
	{
		return;
	}
	
	bool bShouldHitReact = !stateSystem->HasTag(Tag_Parrying);
	Vec2 force = GetPushbackForce();
	
	// 넉백
	float dir = GetPos().x - source->GetPos().x;
	dir = dir < 0 ? -1.0f : 1.0f;
	
	if (context.damageType == EDamageType::SuperHeavy)
	{
		if (stateSystem->HasTag(Tag_Parrying))
		{
			abilitySystem->CancelAbilitiesWithTag(Tag_Parrying);
			bShouldHitReact = true;
		}
		force *= 1.6f;
	}
	else if (context.damageType == EDamageType::Heavy)
	{
		if (stateSystem->HasTag(Tag_Parrying))
		{
			// 옆으로만 밀려남
			SetForward(-dir);
			rigidbody->SetVelocity(Vec2(2 * force.x * dir, 0.f));
		}
	}
	
	if (bShouldHitReact)
	{
		abilitySystem->TryActivateAbility(EAbility::HitReact);
		// 넉백 적용
		SetForward(-dir);
		rigidbody->SetVelocity(Vec2(force.x * dir, -force.y));
		
		if (context.damageType == EDamageType::SuperHeavy || context.damageType == EDamageType::Heavy)
		{
			SFX->PlayOnce(SFXKey::PlayerHeavyDamage);	
		}
	}
	
	// Spawn VFX
	Vec2 spawnPos = context.hitResult.hitCenter;
	int spawnDirection = source->GetForward();
	
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
		
	if (!CAMERA->IsShaking())
	{
		CAMERA->Shake(ShakePreset::Light);	
	}

	// Apply damage
	float newHP = currentHP - context.value;
	SetCurrentHP(newHP);
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
		GAMEUI->SetPlayerMP(currentMP,maxMP);
	}
}

void CPlayer::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);

	// 작은 collider가 필요한 태그들
	const EStateTag smallColliderTags = Tag_Crouching | Tag_Sliding | Tag_Squashed;
	bool hadSmallCollider = HasAnyTag(oldTags,smallColliderTags);
	bool needsSmallCollider = HasAnyTag(newTags, smallColliderTags);

	// 작은 collider 진입
	if (!hadSmallCollider && needsSmallCollider)
	{
		Vec2 crouchScale = characterScale * Vec2(1.0f, 0.5f);
		Vec2 crouchOffset = colOffset + crouchScale * Vec2(0.0f, 0.5f);
		collider->SetScale(crouchScale);
		collider->SetOffset(crouchOffset);
	}
	// 작은 collider 해제 (모든 관련 태그가 없어졌을 때만)
	else if (hadSmallCollider && !needsSmallCollider)
	{
		collider->SetScale(characterScale);
		collider->SetOffset(colOffset);
	}

	// 착지
	if (TagAdded(oldTags, newTags, Tag_Grounded))
	{
		// 착지 시 점프 공격 소진 태그 리셋
		stateSystem->RemoveTag(Tag_AirAttackExhausted);
	}
}

bool CPlayer::ShouldIgnorePlatform() const
{
	return (stateSystem->HasTag(Tag_Climbing));
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

void CPlayer::CheckLedge(CCollider* other)
{
	if (!other || !collider)
		return;

	// 이미 ledge로 마크되어 있는 경우 early return
	if (bOverlapWithLedge && ledgeId == other->GetID())
		return;

	// 매달리기 조건 판별
	Vec2 otherPos = other->GetPos();
	Vec2 otherHalf = other->GetScale() * 0.5f;
	float otherTop = otherPos.y - otherHalf.y;

	Vec2 playerPos = collider->GetPos();
	float playerCenterY = playerPos.y;

	if (playerCenterY < otherTop)
		return;

	// 이미 겹쳐있는 다른 Ledge가 있고 해당 ledge보다 낮으면 갱신 x
	if (bOverlapWithLedge && otherTop > ledgeTop)
	{
		return;
	}

	// 방향 계산: ledge가 플레이어 기준 왼쪽(-1) 또는 오른쪽(1)
	int direction = (otherPos.x > playerPos.x) ? 1 : -1;

	// CLedge의 절벽 방향 확인
	CLedge* ledge = dynamic_cast<CLedge*>(other->GetOwner());
	if (ledge)
	{
		int cliffDir = ledge->GetCliffDirection();
		if (cliffDir != 0 && cliffDir != -direction)
		{
			return;
		}
	}

	bOverlapWithLedge = true;
	ledgeId = other->GetID();
	ledgeTop = otherTop;
	ledgeX = otherPos.x;
	ledgeDirection = direction;
}

void CPlayer::ClearLedge()
{
	bOverlapWithLedge = false;
	ledgeId = 0;
	ledgeTop = -FLT_MAX;
	ledgeX = -FLT_MAX;
	ledgeDirection = 0;
}
