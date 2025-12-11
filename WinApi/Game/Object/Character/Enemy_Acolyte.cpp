#include "pch.h"
#include "Enemy_Acolyte.h"

#include "Game/Data/EnemyAnimData.h"
#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Ability/Common/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_AcolyteAttack.h"
#include "Game/Component/AIController.h"
#include "Game/Component/StatComponent.h"


Enemy_Acolyte::Enemy_Acolyte()
{
	name = TEXT("Acorite");
}

void Enemy_Acolyte::Init()
{
    Enemy::Init();

    // Stats
    statComponent->InitStat(EStatType::HP, MAX_HP);
	statComponent->InitStat(EStatType::AttackPower, ATTACK_POWER);
	
    // Collider
    collider->SetScale(Vec2(32, 66));
    collider->SetOffset(Vec2(0, -33));
    collider->SetLayer((UINT)ELayer::Monster);
	
    // Abilities
    AddAbility<Ability_AcolyteAttack>(EAbility::Attack);
    AddAbility<Ability_HitReaction>(EAbility::HitReact);
    AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);
	AddAbility<Ability_Die>(EAbility::Die);
	
    // Animations
    for (const auto& anim : EnemyAcolyteAnimData::GetAnimations())
    {
        AddAnimation(anim.key, anim.path, anim.repeat);
    }
	
	// AI 설정
	AIConfig config;
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