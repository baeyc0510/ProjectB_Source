#pragma once
#include "ResourceTypes.h"
#include "Game/Data/AnimKey.h"

struct EnemyAcolyteAnimData
{
    static const vector<AnimationEntry>& GetAnimations()
    {
        static const vector<AnimationEntry> animations = {
            {AnimKey::Idle, TEXT("Animations/Enemy/acolite_idle_anim.json"), true},
            {AnimKey::Attack, TEXT("Animations/Enemy/acolyte_attack_anim.json"), false},
            {AnimKey::Hit, TEXT("Animations/Enemy/acolyte_get_hit_anim.json"), false},
            {AnimKey::ParryHit, TEXT("Animations/Enemy/acolyte_parry_reaction_anim.json"), false},
            {AnimKey::Walk, TEXT("Animations/Enemy/acolite_walking_anim.json"), true},
            {AnimKey::Dead, TEXT("Animations/Enemy/acolyte_death_anim.json"), false},
        };
        return animations;
    }
};

struct EnemyStonerAnimData
{
    static const vector<AnimationEntry>& GetAnimations()
    {
        static const vector<AnimationEntry> animations = {
            {AnimKey::Idle, TEXT("Animations/Enemy/stoner_idle_anim.json"), true},
            {AnimKey::StartAggro, TEXT("Animations/Enemy/stoner_rising_anim.json"), false},
            {AnimKey::Attack, TEXT("Animations/Enemy/stoner_attack_anim.json"), false},
            {AnimKey::Turnaround, TEXT("Animations/Enemy/stoner_leftright_anim.json"), false},
            {AnimKey::Dead, TEXT("Animations/Enemy/stoner_death_anim.json"), false},
        };
        return animations;
    }
};