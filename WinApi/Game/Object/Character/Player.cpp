#include "pch.h"
#include "Player.h"

#include "Game/Data/VFXKeys.h"
#include "Game/Data/PlayerAnimData.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Ability/Common/Ability_HitReaction.h"
#include "Game/Manager/SFXManager.h"
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
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/AbilitySystem.h"
#include "Game/Component/CharacterMovement.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/VFXManager.h"
#include "Game/Object/VFXObject.h"

/*~ Initialization ~*/

Player::Player()
{
	name = TEXT("플레이어");
	isPersistent = true;
}

Player::~Player()
{

}

void Player::Init()
{
	Character::Init();
	
	// Rigidbody
	rigidbody->SetGravityScale(PLAYER_GRAVITY_SCALE);

	// Collider
	characterScale = Vec2(CHARACTER_WIDTH, CHARACTER_HEIGHT);
	colOffset = Vec2(0, COLLIDER_OFFSET_Y);
	collider->SetScale(characterScale);
	collider->SetOffset(colOffset);
	collider->SetLayer((UINT)ELayer::Player);

	// Movement (Player 기본 설정: 드롭다운 가능, 엣지 블로킹 없음)
	MovementConfig moveConfig;
	moveConfig.maxSlopeAngle = MAX_SLOPE_ANGLE;
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
	AddAbility<Ability_Die>(EAbility::Die);
	
	// Animations
	for (const auto& anim : PlayerAnimData::GetAnimations())
	{
		AddAnimation(anim.key, anim.path, anim.repeat);
	}
	
	// 초기 스탯값 적용
	InitStartupStats();
}

/*~ Lifecycle ~*/

void Player::OnEnable()
{
	Character::OnEnable();

	// 상태 초기화 (씬 전환 시)
	bWasMovingInput = false;
	prevVelocity = Vec2(0, 0);

	stateSystem->RemoveTagAll(Tag_Moving);
	rigidbody->SetVelocity(Vec2(0, 0));
	movement->SetGrounded(false);
	abilitySystem->CancelAbilitiesWithTag(Tag_AbilityAnimation);
	abilitySystem->CancelAbilitiesWithTag(Tag_Moving);
}

void Player::Update()
{
	if (bIsDown)
		return;
	
	Character::Update();
	
	ProcessActiveInput();
	UpdatePlayerStates();
	ProcessPassiveAbilities();
	UpdateAnimation();
}

void Player::LateUpdate()
{
	CheckVelocitySignChanged();
}

/*~ Input Processing ~*/

void Player::ProcessActiveInput()
{
	HandleCombatInput();
	HandleActionInput();
}

void Player::HandleCombatInput()
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

void Player::HandleActionInput()
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

/*~ State Updates ~*/

void Player::UpdatePlayerStates()
{
	UpdateMovementState();
	UpdateStates();
	UpdateLedgeState();
}

void Player::UpdateMovementState()
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

void Player::UpdateLedgeState()
{
	bool bIsFalling = rigidbody->GetVelocity().y > 0;
	bool bCanMove = !stateSystem->HasTag(Tag_BlockMovement);

	if (!ledgeHelper.IsOverlappingLedge() || !bIsFalling || !bCanMove)
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
		return;
	}

	// 방향 체크: 바라보는 방향과 ledge 방향이 일치해야 함
	if (GetForward() != ledgeHelper.GetLedgeDirection())
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
		return;
	}

	Vec2 playerPos = collider->GetPos();
	Vec2 playerHalfScale = collider->GetScale() * 0.5f;
	float playerTop = playerPos.y - playerHalfScale.y;
	float checkY = playerTop + LEDGE_CLIMB_THRESHOLD;

	// 기준점이 ledgeTop보다 낮은 경우 CanClimb
	if (checkY < ledgeHelper.GetLedgeTop())
	{
		stateSystem->AddTagUnique(Tag_CanClimbLedge);
	}
	else
	{
		stateSystem->RemoveTag(Tag_CanClimbLedge);
	}
}

void Player::ProcessPassiveAbilities()
{
	abilitySystem->TryActivateAbility(EAbility::HangOnLedge); // 매달리기
}

/*~ Animation ~*/

void Player::HandleAnimationEvent(EGameEvent event)
{
	Character::HandleAnimationEvent(event);
	if (event == EGameEvent::Footstep)
	{
		OnFootstep();
	}
}

void Player::OnFootstep()
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

void Player::UpdateAnimation()
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

void Player::Render()
{
	Character::Render();
}

void Player::OnDisable()
{
	Character::OnDisable();
}

void Player::Release()
{
	Character::Release();
}

/*~ Collision ~*/

void Player::OnCollisionEnter(Collider* other)
{
	Character::OnCollisionEnter(other);
}

void Player::OnCollisionStay(Collider* other)
{
	ELayer layer = static_cast<ELayer>(other->GetLayer());
	if (layer == ELayer::Ledge)
	{
		Vec2 playerHalfScale = collider->GetScale() * 0.5f;
		ledgeHelper.CheckLedge(other, GetPos(), playerHalfScale);
	}

	Character::OnCollisionStay(other);
}

void Player::OnCollisionExit(Collider* other)
{
	// 설정한 ledge에서 벗어난 경우 ledge정보 초기화
	ELayer layer = static_cast<ELayer>(other->GetLayer());
	if (layer == ELayer::Ledge && ledgeHelper.ShouldClearOnExit(other))
	{
		ledgeHelper.ClearLedge();
	}

	Character::OnCollisionExit(other);
}

/*~ Combat ~*/

void Player::OnDamage(GameObject* source, const CombatContext& context)
{
	if (stateSystem->HasTag(Tag_Dead))
		return;

	abilitySystem->TriggerEvent(EGameEvent::Hit, source);

	if (context.value <= 0.0001f)
		return;

	// 넉백 방향 계산
	float dir = GetPos().x - source->GetPos().x;
	dir = dir < 0 ? -1.0f : 1.0f;

	// 가드 상호작용 처리
	Vec2 force = GetPushbackForce();
	bool bShouldHitReact = ProcessGuardInteraction(context.damageType, dir, force);

	// 피격 반응 적용
	if (bShouldHitReact)
	{
		ApplyHitReaction(dir, force, context.damageType);
	}

	// VFX 및 카메라 이펙트
	SpawnPlayerDamageVFX(context, source->GetForward());
	if (!CAMERA->IsShaking())
	{
		CAMERA->Shake(ShakePreset::Light);
	}

	// 데미지 적용
	statComponent->TakeDamage(context.value);
}

bool Player::ProcessGuardInteraction(EDamageType damageType, float dir, Vec2& outForce)
{
	bool bIsGuarding = stateSystem->HasTag(Tag_Guard);
	bool bShouldHitReact = !bIsGuarding;

	if (damageType == EDamageType::SuperHeavy)
	{
		// 슈퍼헤비: 가드 파괴
		if (bIsGuarding)
		{
			abilitySystem->CancelAbilitiesWithTag(Tag_Guard);
			bShouldHitReact = true;
		}
		outForce *= SUPER_HEAVY_KNOCKBACK_MULT;
	}
	else if (damageType == EDamageType::Heavy)
	{
		// 헤비: 가드 중이면 밀려남만
		if (bIsGuarding)
		{
			SetForward(-dir);
			rigidbody->SetVelocity(Vec2(HEAVY_GUARD_PUSHBACK_MULT * outForce.x * dir, 0.f));
		}
	}

	return bShouldHitReact;
}

void Player::ApplyHitReaction(float dir, Vec2 force, EDamageType damageType)
{
	abilitySystem->TryActivateAbility(EAbility::HitReact);

	// 넉백 적용
	SetForward(-dir);
	rigidbody->SetVelocity(Vec2(force.x * dir, -force.y));

	// 무거운 공격 사운드
	if (damageType == EDamageType::SuperHeavy || damageType == EDamageType::Heavy)
	{
		SFX->PlayOnce(SFXKey::PlayerHeavyDamage);
	}
}

void Player::SpawnPlayerDamageVFX(const CombatContext& context, int spawnDirection)
{
	Vec2 spawnPos = context.hitResult.hitCenter;

	// Hit VFX
	if (VFXObject* vfx = VFX->CreateVFX(GetPlayerHitVfxKey(context.damageType), spawnPos, spawnDirection))
	{
		vfx->PlayVFX();
	}

	// Blood VFX
	if (VFXObject* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
	{
		vfx->PlayVFX();
	}
}

void Player::InitStartupStats()
{
	pushbackForce = Vec2(PUSHBACK_FORCE_X, PUSHBACK_FORCE_Y);
	
	// 스탯 초기화
	statComponent->InitStat(EStatType::HP, MAX_HP);
	statComponent->InitStat(EStatType::MP, MAX_MP);
	statComponent->InitStat(EStatType::Flask, static_cast<float>(MAX_FLASK));
	statComponent->InitStat(EStatType::JumpForce, JUMP_FORCE, JUMP_FORCE);
	statComponent->InitStat(EStatType::AttackPower, ATTACK_POWER, ATTACK_POWER);
}

/*~ State Events ~*/

void Player::OnStatChanged(EStatType type, float& current, float& max)
{
	switch (type)
	{
	case EStatType::HP:
		GAMEUI->SetPlayerHP(current, max);
		break;
	case EStatType::MP:
		GAMEUI->SetPlayerMP(current, max);
		break;
	case EStatType::Flask:
		GAMEUI->SetPlayerFlask(static_cast<int>(current), static_cast<int>(max));
		break;
	default:
		break;
	}
	
	Character::OnStatChanged(type, current, max);
}

Vec2 Player::GetKnockbackVelocity(GameObject* source, const CombatContext& context)
{
	Vec2 direction = GetPos() - source->GetPos();
	float dirX = direction.x > 0 ? 1.f : -1.f;
	return Vec2(KNOCKBACK_POWER * dirX, KNOCKBACK_POWER);
}

wstring Player::GetPlayerHitVfxKey(EDamageType damageType)
{
	return VFXKey::PlayerHit;
}

void Player::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	Character::OnStateChanged(oldTags, newTags);

	// 작은 collider가 필요한 태그들
	const EStateTag smallColliderTags = Tag_Crouching | Tag_Sliding | Tag_Squashed;
	bool hadSmallCollider = HasAnyTag(oldTags,smallColliderTags);
	bool needsSmallCollider = HasAnyTag(newTags, smallColliderTags);

	// 작은 collider 진입
	if (!hadSmallCollider && needsSmallCollider)
	{
		Vec2 crouchScale = characterScale * Vec2(1.0f, CROUCH_HEIGHT_SCALE);
		Vec2 crouchOffset = colOffset + crouchScale * Vec2(0.0f, CROUCH_HEIGHT_SCALE);
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

bool Player::ShouldIgnorePlatform() const
{
	return (stateSystem->HasTag(Tag_Climbing));
}

void Player::CheckVelocitySignChanged()
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

