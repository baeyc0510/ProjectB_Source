#include "pch.h"
#include "CEnemy_Acorite.h"

#include "Game/Ability/Ability_HitReaction.h"
#include "Game/Ability/Monster/Ability_MonsterAttack.h"

CEnemy_Acorite::CEnemy_Acorite()
{
	name = TEXT("Acorite");
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
    AddAbility<Ability_HitReaction>(EAbility::HitReact);
    AddAbility<Ability_ParryHitReaction>(EAbility::ParryHit);
	
    // Animations
    AddAnimation(AnimKey::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true);
    AddAnimation(AnimKey::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false);
    AddAnimation(AnimKey::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false);
    AddAnimation(AnimKey::ParryHit, TEXT("Animations/Enemy/acolyte_parry_reaction_anim.json"), false);
    AddAnimation(AnimKey::Walk, TEXT("Animations/Enemy/acolite_walking_anim.json"), true);
}