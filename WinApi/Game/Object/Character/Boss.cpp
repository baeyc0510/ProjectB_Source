#include "pch.h"
#include "Boss.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/BossAIController.h"
#include "Game/Component/CharacterMovement.h"
#include "Game/Manager/GameUIManager.h"


Boss::Boss()
{
	name = TEXT("Boss");
}

Boss::~Boss()
{
}

void Boss::Init()
{
	Character::Init();

	// Movement 설정 (보스는 엣지에서 블로킹, 벽에서 블로킹)
	FMovementConfig moveConfig;
	moveConfig.maxSlopeAngle = 50.0f;
	moveConfig.bCanDropThrough = false;
	moveConfig.bBlockAtEdges = true;
	moveConfig.bFlipDirectionAtEdge = false;
	moveConfig.bFlipDirectionAtWall = false;
	movement->SetConfig(moveConfig);

	// StatComponent 이벤트 바인딩
	statComponent->OnStatChanged.Add([this](EStatType type, float current, float max) {
		OnStatChanged(type, current, max);
	});

	// Boss AI Controller
	bossAI = new BossAIController();
	AddChild(bossAI);
}

void Boss::OnEnable()
{
	Character::OnEnable();
}

void Boss::Update()
{
	Character::Update();

	UpdateStates();

	if (bHasAppeared)
	{
		UpdateBossAI();
		UpdateBossAnimation();
	}
	else if (!bHasEncountered)
	{
		if (CheckEncounterPlayer())
		{
			bHasEncountered = true;
			TriggerAppearance();
		}
	}
	
	animator->SetDirection(GetForward());
}

void Boss::Render()
{
	Character::Render();
}

void Boss::OnDisable()
{
	Character::OnDisable();
	GAMEUI->ShowBossHUD(false);
}

void Boss::Release()
{
	Character::Release();
}

void Boss::TriggerAppearance()
{
	if (bHasAppeared)
		return;

	stateSystem->AddTag(Tag_BossAppearing);
	// 보스 별 연출 효과를 Ability에서 구현
	abilitySystem->TryActivateAbility(EAbility::Boss_Appear);
}

void Boss::OnAppearanceComplete()
{
	bHasAppeared = true;
	stateSystem->RemoveTag(Tag_BossAppearing);
	GAMEUI->ShowBossHUD(true);
	// HP 이벤트 강제 발생 (UI 초기 업데이트)
	GAMEUI->SetBossHP(statComponent->GetCurrent(EStatType::HP), statComponent->GetMax(EStatType::HP));
}

void Boss::OnStatChanged(EStatType type, float current, float max)
{
	if (type == EStatType::HP && bHasAppeared)
	{
		GAMEUI->SetBossHP(current, max);
	}
}

void Boss::UpdateBossAnimation()
{
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 기본 Idle
	animator->Play(AnimKey::Idle);
}

void Boss::UpdateBossAI()
{
	// 이동 차단 태그 그룹
	const EStateTag TAG_MOVEMENT_BLOCKED = Tag_StopVelocity | Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement;
	
	// 이동 불가 상태면 정지
	if (stateSystem->HasAnyTag(TAG_MOVEMENT_BLOCKED))
	{
		StopHorizontalMovement();
		return;
	}

	// AI 컨트롤러에서 공격 선택 및 실행
	if (bossAI && bossAI->HasTarget())
	{
		EAbility nextAttack = bossAI->SelectNextAttack();
		if (nextAttack != EAbility::None)
		{
			abilitySystem->TryActivateAbility(nextAttack);
		}
	}
}

void Boss::OnDamage(CGameObject* source, const CombatContext& context)
{
	// Trigger Event
	abilitySystem->TriggerEvent(EGameEvent::Hit, source);

	// Spawn VFX
	SpawnDamageVFX(context, source->GetForward());

	if (context.value > 0.0001f)
	{
		// Apply Damage (OnStatChanged handles UI update)
		statComponent->TakeDamage(context.value);
	}
}

void Boss::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	Character::OnStateChanged(oldTags, newTags);
}

bool Boss::IsAtArenaBoundary(int dir) const
{
	// 아레나 경계가 설정되어 있지 않으면 false
	if (arenaMinX == 0.f && arenaMaxX == 0.f)
		return false;

	float currentX = pos.x;
	return (dir > 0 && currentX >= arenaMaxX) || (dir < 0 && currentX <= arenaMinX);
}
