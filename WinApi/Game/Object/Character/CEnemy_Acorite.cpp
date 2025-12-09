#include "pch.h"
#include "CEnemy_Acorite.h"

#include "Game/Ability/Common/Ability_Die.h"
#include "Game/Ability/Common/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_MonsterAttack.h"


CEnemy_Acorite::CEnemy_Acorite()
{
	name = TEXT("Acorite");
	currentHP = 50;
	maxHP = 50;
}

void CEnemy_Acorite::Init()
{
    CEnemy::Init();
    
    // Collider
    collider->SetScale(Vec2(32, 66));
    collider->SetOffset(Vec2(0, -33));
    collider->SetLayer(ELayer::Monster);
	
    // Abilities
    AddAbility<Ability_MonsterAttack>(EAbility::Attack);
    AddAbility<Ability_HitReaction>(EAbility::Hit);
    AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);
	AddAbility<Ability_Die>(EAbility::Die);
	
    // Animations
    AddAnimation(AnimKey::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true);
    AddAnimation(AnimKey::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false);
    AddAnimation(AnimKey::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false);
    AddAnimation(AnimKey::ParryHit, TEXT("Animations/Enemy/acolyte_parry_reaction_anim.json"), false);
    AddAnimation(AnimKey::Walk, TEXT("Animations/Enemy/acolite_walking_anim.json"), true);
	AddAnimation(AnimKey::Dead, TEXT("Animations/Enemy/acolyte_death_anim.json"), false);
}