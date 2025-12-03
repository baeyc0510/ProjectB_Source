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
#include "Game/Map/CMetaMap.h"
#include "Game/Object/CVFX.h"

CPlayer::CPlayer() : currentHP(0), maxHP(0), currentMP(0), maxMP(0), currentFlask(0), maxFlask(0), prevVelocity(0, 0)
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
	collider = new CCollider();
	standingColScale = Vec2(42, 72);
	standingColOffset = Vec2(0, -36);
	collider->SetScale(standingColScale);
	collider->SetOffset(standingColOffset);
	collider->SetLayer(ELayer::Player);
	AddChild(collider);
	
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
	animator->Play(AnimKey::Idle, false);

	// StateSystem 이벤트 구독
	stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags)
	{
		OnStateChanged(oldTags, newTags);
	});
}

void CPlayer::Update()
{
	CCharacter::Update();
	HandleCombatInput();
	HandleActionInput();
	UpdateMovement();
	// UpdateMetaCollision();  // 콜라이더 기반 충돌로 대체됨 (OnCollisionStay)
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
		if (INPUT->ButtonDown('W') || INPUT->ButtonDown('S'))
		{
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
		abilitySystem->CancelAbility(EAbility::Crouch);
	}

	// JUMP
	if (INPUT->ButtonDown(VK_SPACE))
	{
		abilitySystem->TryActivateAbility(EAbility::Jump);
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
		SetPos(Vec2(ladderX,pos.y));

		// 상하 이동
		if (INPUT->ButtonStay('W'))
		{
			velocity.y = -CLIMB_SPEED;
		}
		else if (INPUT->ButtonStay('S'))
		{
			velocity.y = CLIMB_SPEED;
		}

		rigidbody->SetVelocity(velocity);
		animator->SetReverse(velocity.y > 0.f); // 하강시 역재생
		return;
	}

	// 일반 이동
	if (INPUT->ButtonStay('A'))
	{
		velocity.x = -MOVE_SPEED;
		SetForward(-1);
		stateSystem->AddTagUnique(Tag_Moving);
	}
	else if (INPUT->ButtonStay('D'))
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
	else if (stateSystem->HasTag(Tag_Moving))
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

void CPlayer::ProcessMetaCollision(CollisionContext& ctx)
{
	CCharacter::ProcessMetaCollision(ctx);
	ProcessLadderOverlap(ctx);
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
	// 앉기
	if (TagAdded(oldTags, newTags, Tag_Crouching) || TagAdded(oldTags, newTags, Tag_Sliding))
	{
		Vec2 crouchScale = standingColScale * Vec2(1.0f, 0.5f);
		Vec2 crouchOffset = standingColOffset + crouchScale * Vec2(0.0f, 0.5f);
		collider->SetScale(crouchScale);
		collider->SetOffset(crouchOffset);
	}
	// 앉기 해제
	else if (TagRemoved(oldTags, newTags, Tag_Crouching) || TagRemoved(oldTags, newTags, Tag_Sliding))
	{
		collider->SetScale(standingColScale);
		collider->SetOffset(standingColOffset);
	}
	
	// 착지
	if (TagAdded(oldTags, newTags, Tag_Grounded))
	{
		// 착지 이벤트 트리거
		abilitySystem->TriggerEvent(EGameEvent::Landed);
		// 착지 시 점프 공격 소진 태그 리셋
		stateSystem->RemoveTag(Tag_AirAttackExhausted);
		// 착지 시 사다리 타기 해제
		stateSystem->RemoveTag(Tag_Climbing);
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

// 사다리 오버랩 체크 (비활성화 - 콜라이더 기반 시스템으로 대체 예정)
void CPlayer::ProcessLadderOverlap(CollisionContext& ctx)
{
	// TODO: 콜라이더 기반 사다리 체크로 대체
	return;

	/*
	static constexpr int OVERLAP_WIDTH = 20;

	int centerX = (int)ctx.pixelCenter.x;
	int centerY = (int)ctx.pixelCenter.y;
	int feetY = (int)(ctx.pixelCenter.y + ctx.halfHeight);

	// 캐릭터 중심에 사다리가 있는지 체크
	bool bOverlapCenter = false;
	bool bOverlapFeet = false;
	bool bOverlap = false;

	int checkX = centerX - OVERLAP_WIDTH / 2;
	int checkY = (centerY + feetY) / 2;

	for (int i = 0; i < OVERLAP_WIDTH; i++)
	{
		bOverlapCenter = bOverlapCenter || ctx.metaMap->IsLadder(checkX,centerY);
		bOverlapFeet = bOverlapFeet || ctx.metaMap->IsLadder(checkX,feetY);

		if (stateSystem->HasTag(Tag_Climbing))
		{
			bOverlap = ctx.metaMap->IsLadder(checkX,checkY);
		}
		else
		{
			bOverlap = bOverlapCenter ||  bOverlapFeet;
		}

		if (bOverlap)
		{
			ladderX = MAP->PixelToWorld(Vec2(checkX,centerY)).x;
			break;
		}
		checkX ++;
	}

	if (bOverlap)
	{
		stateSystem->AddTagUnique(Tag_CanClimb);
	}
	else
	{
		stateSystem->RemoveTag(Tag_CanClimb);

		// 사다리 밖으로 나가면 타기 상태 해제
		if (stateSystem->HasTag(Tag_Climbing))
		{
			stateSystem->RemoveTag(Tag_Climbing);
			rigidbody->UseGravity(true);

			if (bOverlapFeet)
			{
				Vec2 worldPos = MAP->PixelToWorld(Vec2(checkX,checkY));
				SetPos(Vec2(ladderX, worldPos.y));

				stateSystem->AddTag(Tag_AbilityAnimation);
				stateSystem->AddTag(Tag_BlockMovement);
				stateSystem->AddTag(Tag_StopVelocity);
				animator->Play(AnimKey::CrouchUp, true, [this]()
				{
					stateSystem->RemoveTag(Tag_AbilityAnimation);
					stateSystem->RemoveTag(Tag_BlockMovement);
					stateSystem->RemoveTag(Tag_StopVelocity);
				});
			}
		}
	}
	*/
}
