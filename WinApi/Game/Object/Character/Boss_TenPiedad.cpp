#include "pch.h"
#include "Boss_TenPiedad.h"

#include "Game/Data/BossAnimData.h"
#include "Game/Component/BossAIController.h"

// Boss Abilities
#include "Player.h"
#include "Game/Ability/Boss/Ability_BossAppear.h"
#include "Game/Ability/Boss/Ability_PiedadSlash.h"
#include "Game/Ability/Boss/Ability_PiedadSpit.h"
#include "Game/Ability/Boss/Ability_PiedadStomp.h"
#include "Game/Ability/Boss/Ability_PiedadGroundSmash.h"
#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Manager/MapManager.h"
#include "Game/Manager/SFXManager.h"


Boss_TenPiedad::Boss_TenPiedad()
{
	name = TEXT("Ten Piedad");
}

wstring Boss_TenPiedad::GetIntroSoundKey() const
{
	return SFXKey::PiedadWakeUp;
}

void Boss_TenPiedad::Init()
{
	Boss::Init();

	// Collider
	collider->SetScale(Vec2(80, 150));
	collider->SetOffset(Vec2(0, -75));
	collider->SetLayer((UINT)ELayer::Monster);

	// 이름 이미지
	nameImg = LOADIMAGE(L"Ten_Piedad_Name", L"Image/Sheet/boss_tenpiedad_name.bmp");
	
	RegisterAnimations();
	RegisterAbilities();
	ConfigureAI();

	// Stats
	statComponent->InitStat(EStatType::HP, 5000.f);

	// 등장 장면 첫 프레임에 고정
	animator->Play(AnimKey::BossAppear,true);
	animator->Stop();
	SetForward(-1);
}

void Boss_TenPiedad::RegisterAnimations()
{
	for (const auto& anim : BossTenPiedadAnimData::GetAnimations())
	{
		AddAnimation(anim.key, anim.path, anim.repeat);
	}
}

void Boss_TenPiedad::RegisterAbilities()
{
	// 보스 공격 어빌리티
	AddAbility<Ability_PiedadSlash>(EAbility::Boss_Slash);
	AddAbility<Ability_PiedadSpit>(EAbility::Boss_Spit);
	AddAbility<Ability_PiedadStomp>(EAbility::Boss_Stomp);
	AddAbility<Ability_PiedadGroundSmash>(EAbility::Boss_GroundSmash);
	AddAbility<Ability_BossAppear>(EAbility::Boss_Appear);
	AddAbility<Ability_Die>(EAbility::Die);
}

void Boss_TenPiedad::ConfigureAI()
{
	if (!bossAI)
		return;

	// 공격 패턴 데이터 등록 (ability, minRange, maxRange, weight)
	bossAI->RegisterAttack(EAbility::Boss_Slash, 0.f, Config::Attack::SlashRange, 1.5f);
	bossAI->RegisterAttack(EAbility::Boss_Stomp, 0.f, Config::Attack::StompRange, 1.0f);
	bossAI->RegisterAttack(EAbility::Boss_Spit, Config::Attack::SpitMinRange, 400.f, 1.0f);
	bossAI->RegisterAttack(EAbility::Boss_GroundSmash, 0.f, Config::Attack::GroundSmashRange, 0.8f);

	// 결정 간격 설정
	bossAI->SetDecisionInterval(1.0f);

	// 추격 설정
	BossChaseConfig chaseConfig;
	chaseConfig.chaseSpeed = Config::Chase::Speed;
	chaseConfig.chaseRange = Config::Chase::Range;
	chaseConfig.stopRange = Config::Chase::StopRange;
	chaseConfig.bCanChase = true;
	bossAI->SetChaseConfig(chaseConfig);

	// arena 설정
	arenaMinX = MAP->PixelToWorld(Vec2(0,0)).x;
	arenaMaxX = MAP->PixelToWorld(Vec2(1950,0)).x;
}

void Boss_TenPiedad::OnAppearanceComplete()
{
	Boss::OnAppearanceComplete();
	
	GAMEUI->SetBossName(nameImg);
	
	// 등장 완료 후 idle 애니메이션 시작
	animator->Play(AnimKey::Idle);
	
	// BGM 재생
	SFX->PlayBGM(SFXKey::BGM_Piedad, 0.8f);
}

void Boss_TenPiedad::UpdateBossAnimation()
{
	// 어빌리티 또는 턴어라운드 중이면 애니메이션 오버라이드 하지 않음
	if (stateSystem->HasTag(Tag_AbilityAnimation) || bIsTurningAround)
		return;

	// 추격 중이면 Walk
	if (bIsChasing)
	{
		animator->Play(AnimKey::Walk);
	}
	else
	{
		animator->Play(AnimKey::Idle);
	}
}

bool Boss_TenPiedad::CheckEncounterPlayer()
{
	if (!bossAI)
		return false;

	float distance = bossAI->GetDistanceToTarget();
	return distance < Config::Encounter::Range;
}

void Boss_TenPiedad::OnDamage(GameObject* source, const CombatContext& context)
{
	if (!bHasAppeared)
		return;
	
	if (stateSystem->HasTag(Tag_Dead))
		return;
	
	Boss::OnDamage(source, context);
	
	// Hitstop + Camera Shake
	TIMER->SetTimeScale(0.0f, 0.05f);
	CAMERA->Shake(ShakePreset::Medium);
}

void Boss_TenPiedad::OnDieStart()
{
	Boss::OnDieStart();
	
	// SFX 효과
	SFX->StopBGM();
	SFX->PlayOnce(SFXKey::PiedadDeath);
	SFX->PlayOnce(SFXKey::PiedadDeathVoice);
}

void Boss_TenPiedad::OnDieComplete()
{
	Boss::OnDieComplete();
	
	// 클리어 연출
	SFX->PlayOnce(SFXKey::ClearBoss);
	GAMEUI->OpenUI(EOverlayUI::BossDefeat);
	GAMEUI->ShowPlayerHUD(false);
	GAMEUI->ShowBossHUD(false);
	CAMERA->FadeOut(1.5f);
	TIMER->SetTimer([this]()
	{
		CAMERA->FadeIn(2.5f);
		TIMER->SetTimer([this]()
		{
			GAMEUI->CloseUI();
			GAMEUI->ShowPlayerHUD(true);
		},2.5f);
	},1.5f);
}

void Boss_TenPiedad::UpdateBossAI()
{
	// 이동 차단 태그 그룹
	const EStateTag TAG_MOVEMENT_BLOCKED = Tag_StopVelocity | Tag_Hit | Tag_Stunned | Tag_Attacking | Tag_BlockMovement;
	
	// 이동 불가 상태면 정지
	if (stateSystem->HasAnyTag(TAG_MOVEMENT_BLOCKED))
	{
		StopHorizontalMovement();
		bIsChasing = false;
		bIsTurningAround = false;
		return;
	}

	// 턴어라운드 중이면 대기
	if (bIsTurningAround)
	{
		StopHorizontalMovement();
		return;
	}

	if (!bossAI || !bossAI->HasTarget() || IsTargetDead())
		return;

	// 플레이어가 뒤에 있으면 턴어라운드 먼저
	if (NeedsTurnaround())
	{
		StartTurnaround();
		return;
	}

	// 공격 가능하면 공격
	EAbility nextAttack = bossAI->SelectNextAttack();
	if (nextAttack != EAbility::None)
	{
		bIsChasing = false;
		StopHorizontalMovement();
		abilitySystem->TryActivateAbility(nextAttack);
		return;
	}

	// 공격 불가하면 추격
	if (bossAI->ShouldChase())
	{
		UpdateChaseMovement();
		return;
	}

	// 추격도 불가하면 정지
	bIsChasing = false;
	StopHorizontalMovement();
}

void Boss_TenPiedad::UpdateChaseMovement()
{
	int dir = GetForward();  // 현재 보스 방향

	// 아레나 경계 체크
	if (IsAtArenaBoundary(dir))
	{
		bIsChasing = false;
		StopHorizontalMovement();
		return;
	}

	// 이동
	bIsChasing = true;
	rigidbody->SetVelocity(Vec2(bossAI->GetChaseSpeed() * dir, rigidbody->GetVelocity().y));
}

bool Boss_TenPiedad::NeedsTurnaround() const
{
	if (!bossAI || !bossAI->HasTarget())
		return false;

	// 현재 보스 방향과 플레이어 방향 비교
	int targetDir = bossAI->GetDirectionToTarget();
	int currentDir = GetForward();

	// 방향이 다르면 턴어라운드 필요
	return targetDir != currentDir;
}

void Boss_TenPiedad::StartTurnaround()
{
	bIsTurningAround = true;
	bIsChasing = false;
	StopHorizontalMovement();

	// 턴어라운드 애니메이션 재생, 완료 시 콜백
	animator->Play(AnimKey::Turnaround, false, BIND(this,OnTurnaroundComplete));
	
	SFX->PlayOnce(SFXKey::PiedadTurn);
}

void Boss_TenPiedad::OnTurnaroundComplete()
{
	// 방향 전환
	int newDir = bossAI->GetDirectionToTarget();
	SetForward(newDir);

	// 턴어라운드 완료
	bIsTurningAround = false;
}
