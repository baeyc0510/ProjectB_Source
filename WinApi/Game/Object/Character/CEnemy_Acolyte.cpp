#include "pch.h"
#include "CEnemy_Acolyte.h"

#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Ability/Common/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_AcolyteAttack.h"
#include "Game/Component/CAIController.h"
#include "Game/Component/CStatComponent.h"


CEnemy_Acolyte::CEnemy_Acolyte()
{
	name = TEXT("Acorite");
}

void CEnemy_Acolyte::Init()
{
    CEnemy::Init();

    // Stats
    statComponent->InitStat(EStatType::HP, 50.f);

    // Collider
    collider->SetScale(Vec2(32, 66));
    collider->SetOffset(Vec2(0, -33));
    collider->SetLayer(ELayer::Monster);
	
    // Abilities
    AddAbility<Ability_AcolyteAttack>(EAbility::Attack);
    AddAbility<Ability_HitReaction>(EAbility::HitReact);
    AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);
	AddAbility<Ability_Die>(EAbility::Die);
	
    // Animations
    AddAnimation(AnimKey::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true);
    AddAnimation(AnimKey::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false);
    AddAnimation(AnimKey::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false);
    AddAnimation(AnimKey::ParryHit, TEXT("Animations/Enemy/acolyte_parry_reaction_anim.json"), false);
    AddAnimation(AnimKey::Walk, TEXT("Animations/Enemy/acolite_walking_anim.json"), true);
	AddAnimation(AnimKey::Dead, TEXT("Animations/Enemy/acolyte_death_anim.json"), false);
	
	// AI 설정
	FAIConfig config;
	config.attackRange = 70.0f;
	config.detectRange = 200.0f;
	config.detectRangeY = 50.0f;
	config.loseTargetRange = 300.0f;
	config.patrolRange = 300.0f;
	config.patrolSpeed = 50.0f;
	config.chaseSpeed = 100.0f;
	config.requireFacingTarget = true;
	
	aiController->SetConfig(config);
}