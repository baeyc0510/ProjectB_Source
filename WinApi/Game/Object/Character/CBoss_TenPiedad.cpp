#include "pch.h"
#include "CBoss_TenPiedad.h"

#include "Game/AnimKey.h"
#include "Game/Ability/Ability_HitReaction.h"
#include "Game/Component/CBossAIController.h"

// Boss Abilities
#include "Game/Ability/Boss/Ability_BossSlash.h"
#include "Game/Ability/Boss/Ability_BossSpit.h"
#include "Game/Ability/Boss/Ability_BossStomp.h"
#include "Game/Ability/Boss/Ability_BossGroundSmash.h"

CBoss_TenPiedad::CBoss_TenPiedad()
{
	name = TEXT("Ten Piedad");
}

void CBoss_TenPiedad::Init()
{
	CBoss::Init();

	// Collider (보스는 더 큰 히트박스)
	collider->SetScale(Vec2(80, 150));
	collider->SetOffset(Vec2(0, -75));
	collider->SetLayer(ELayer::Monster);

	RegisterAnimations();
	RegisterAbilities();
	ConfigureAI();
}

void CBoss_TenPiedad::RegisterAnimations()
{
	// 기본 상태
	AddAnimation(AnimKey::BossIdle, TEXT("Animations/Boss/ten_piedad_idle_anim.json"), true);
	AddAnimation(AnimKey::BossWalk, TEXT("Animations/Boss/ten_piedad_walk_anim.json"), true);
	AddAnimation(AnimKey::BossWalkToIdle, TEXT("Animations/Boss/ten_piedad_walk_to_idle_anim.json"), false);
	AddAnimation(AnimKey::BossTurnaround, TEXT("Animations/Boss/ten_piedad_turnaround_anim.json"), false);

	// 등장
	AddAnimation(AnimKey::BossAppear, TEXT("Animations/Boss/ten_piedad_appear_anim.json"), false);

	// 공격
	AddAnimation(AnimKey::BossSlash, TEXT("Animations/Boss/ten_piedad_slash_anim.json"), false);
	AddAnimation(AnimKey::BossSpitStart, TEXT("Animations/Boss/ten_piedad_spit_start_anim.json"), false);
	AddAnimation(AnimKey::BossSpitLoop, TEXT("Animations/Boss/ten_piedad_spit_loop_anim.json"), false);
	AddAnimation(AnimKey::BossSpitToIdle, TEXT("Animations/Boss/ten_piedad_spit_to_idle_anim.json"), false);
	AddAnimation(AnimKey::BossStomp, TEXT("Animations/Boss/ten_piedad_stomp_anim.json"), false);
	AddAnimation(AnimKey::BossGroundSmash, TEXT("Animations/Boss/ten_piedad_ground_smash_anim.json"), false);
	AddAnimation(AnimKey::BossGroundSmashToIdle, TEXT("Animations/Boss/ten_piedad_ground_smash_to_idle_anim.json"), false);

	// 피격
	AddAnimation(AnimKey::Hit, TEXT("Animations/Boss/ten_piedad_hit_anim.json"), false);
	AddAnimation(AnimKey::ParryHit, TEXT("Animations/Boss/ten_piedad_parry_hit_anim.json"), false);
}

void CBoss_TenPiedad::RegisterAbilities()
{
	// 피격 반응
	AddAbility<Ability_HitReaction>(EAbility::Hit);
	AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);

	// 보스 공격 어빌리티
	AddAbility<Ability_BossSlash>(EAbility::Boss_Slash);
	AddAbility<Ability_BossSpit>(EAbility::Boss_Spit);
	AddAbility<Ability_BossStomp>(EAbility::Boss_Stomp);
	AddAbility<Ability_BossGroundSmash>(EAbility::Boss_GroundSmash);
}

void CBoss_TenPiedad::ConfigureAI()
{
	if (!bossAI)
		return;

	// 공격 패턴 등록 (ability, cooldown, minRange, maxRange, weight)
	bossAI->RegisterAttack(EAbility::Boss_Slash, 3.0f, 0.f, SLASH_RANGE, 1.5f);
	bossAI->RegisterAttack(EAbility::Boss_Stomp, 4.0f, 0.f, STOMP_RANGE, 1.0f);
	bossAI->RegisterAttack(EAbility::Boss_Spit, 5.0f, SPIT_MIN_RANGE, 400.f, 1.0f);
	bossAI->RegisterAttack(EAbility::Boss_GroundSmash, 6.0f, 0.f, GROUND_SMASH_RANGE, 0.8f);

	// 결정 간격 설정
	bossAI->SetDecisionInterval(1.0f);
}

void CBoss_TenPiedad::OnAppearanceComplete()
{
	CBoss::OnAppearanceComplete();

	// 등장 완료 후 idle 애니메이션 시작
	animator->Play(AnimKey::BossIdle);
}

void CBoss_TenPiedad::UpdateBossAnimation()
{
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 기본 Idle
	animator->Play(AnimKey::BossIdle);
}
