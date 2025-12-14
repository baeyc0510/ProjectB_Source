#include "pch.h"
#include "Enemy_Stoner.h"

#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Ability/Monster/Ability_StonerAttack.h"
#include "Game/Component/AIController.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/StateSystem.h"
#include "Game/Data/EnemyAnimData.h"

void Enemy_Stoner::Init()
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
    AddAbility<Ability_StonerAttack>(EAbility::Attack);
    AddAbility<Ability_Die>(EAbility::Die);
	
    // Animations
    for (const auto& anim : EnemyStonerAnimData::GetAnimations())
    {
        AddAnimation(anim.key, anim.path, anim.repeat);
    }
	
    // AI 설정
    AIConfig config;
    config.attackRange = 200.0f;
    config.detectRange = 250.0f;
    config.detectRangeY = 250.0f;
    config.loseTargetRange = 350.0f;
    config.patrolRange = 250.0f;
    config.patrolSpeed = 0.0f;
    config.chaseSpeed = 0.0f;
    config.requireFacingTarget = true;
	
    aiController->SetConfig(config);
}

void Enemy_Stoner::UpdateAIActions()
{
	if (bIsInAggroMotion)
		return;
	
	Enemy::UpdateAIActions();
}

void Enemy_Stoner::UpdateAnimation()
{
	animator->SetDirection(GetForward());
	
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;
	
	if (bIsInAggroMotion)
		return;
	
	// 휴면 상태
	if (!stateSystem->HasTag(Tag_HasTarget))
	{
		// 첫프레임에 고정
		animator->Play(AnimKey::StartAggro, true);
		animator->Stop();
		return;
	}
	
	// 기본 상태
	animator->Play(AnimKey::Idle);
}

void Enemy_Stoner::MoveInDirection(int dir, float speed)
{
	// TODO: turnaround
	Enemy::MoveInDirection(dir, speed);
}

void Enemy_Stoner::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
	Enemy::OnStateChanged(oldTags, newTags);
	
	// 죽은 이후로는 처리 X
	if (stateSystem->HasTag(Tag_Dead))
		return;
	
	// 타겟 감지
	if (TagAdded(oldTags, newTags, Tag_HasTarget))
	{
		SetForward(aiController->GetDirectionToTarget());
		bIsInAggroMotion = true;
		animator->SetReverse(false);
		animator->Play(AnimKey::StartAggro, false, BIND(this,OnEndAggroMotion), BIND(this,OnEndAggroMotion));
	}
	// 타겟 잃음
	if (TagRemoved(oldTags, newTags, Tag_HasTarget))
	{
		bIsInAggroMotion = true;
		// 애니메이션 역재생
		animator->SetReverse(true);
		animator->Play(AnimKey::StartAggro, false, BIND(this,OnEndAggroMotion), BIND(this,OnEndAggroMotion));
	}
}

void Enemy_Stoner::OnEndAggroMotion()
{
	bIsInAggroMotion = false;
	animator->SetReverse(false);
}