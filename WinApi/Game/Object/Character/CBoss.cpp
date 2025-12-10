#include "pch.h"
#include "CBoss.h"

#include "Game/AnimKey.h"
#include "Game/Enum.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Component/CBossAIController.h"
#include "Game/Component/CCharacterMovement.h"
#include "Game/Manager/CGameUIManager.h"


CBoss::CBoss()
{
	name = TEXT("Boss");
}

CBoss::~CBoss()
{
}

void CBoss::Init()
{
	CCharacter::Init();

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
	bossAI = new CBossAIController();
	AddChild(bossAI);
}

void CBoss::OnEnable()
{
	CCharacter::OnEnable();
}

void CBoss::Update()
{
	CCharacter::Update();

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

void CBoss::Render()
{
	CCharacter::Render();
}

void CBoss::OnDisable()
{
	CCharacter::OnDisable();
	GAMEUI->ShowBossHUD(false);
}

void CBoss::Release()
{
	CCharacter::Release();
}

void CBoss::TriggerAppearance()
{
	if (bHasAppeared)
		return;

	stateSystem->AddTag(Tag_BossAppearing);
	// 보스 별 연출 효과를 Ability에서 구현
	abilitySystem->TryActivateAbility(EAbility::Boss_Appear);
}

void CBoss::OnAppearanceComplete()
{
	bHasAppeared = true;
	stateSystem->RemoveTag(Tag_BossAppearing);
	GAMEUI->ShowBossHUD(true);
	// HP 이벤트 강제 발생 (UI 초기 업데이트)
	GAMEUI->SetBossHP(statComponent->GetCurrent(EStatType::HP), statComponent->GetMax(EStatType::HP));
}

void CBoss::OnStatChanged(EStatType type, float current, float max)
{
	if (type == EStatType::HP && bHasAppeared)
	{
		GAMEUI->SetBossHP(current, max);
	}
}

void CBoss::UpdateBossAnimation()
{
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 기본 Idle
	animator->Play(AnimKey::Idle);
}

void CBoss::UpdateBossAI()
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

void CBoss::OnDamage(CGameObject* source, const CombatContext& context)
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

void CBoss::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	CCharacter::OnStateChanged(oldTags, newTags);
}

bool CBoss::IsAtArenaBoundary(int dir) const
{
	// 아레나 경계가 설정되어 있지 않으면 false
	if (arenaMinX == 0.f && arenaMaxX == 0.f)
		return false;

	float currentX = pos.x;
	return (dir > 0 && currentX >= arenaMaxX) || (dir < 0 && currentX <= arenaMinX);
}
