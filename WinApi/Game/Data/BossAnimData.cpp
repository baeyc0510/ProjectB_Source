#include "pch.h"
#include "BossAnimData.h"
#include "Game/AnimKey.h"

const std::vector<AnimationEntry>& BossTenPiedadAnimData::GetAnimations()
{
	static const std::vector<AnimationEntry> animations = {
		// Locomotion
		{ AnimKey::Idle,		TEXT("Animations/Ten_Piedad/piedad_idle_anim.json"),	true },
		{ AnimKey::Walk,		TEXT("Animations/Ten_Piedad/piedad_walk.json"),			true },
		{ AnimKey::WalkToIdle,	TEXT("Animations/Ten_Piedad/piedad_walkToIdle.json"),	false },
		{ AnimKey::Turnaround,	TEXT("Animations/Ten_Piedad/piedad_turnaround.json"),	false },

		// Appear
		{ AnimKey::BossAppear,	TEXT("Animations/Ten_Piedad/piedad_appear.json"),		false },

		// Attack
		{ AnimKey::BossSlash,			 TEXT("Animations/Ten_Piedad/piedad_slash_anim.json"),			 false },
		{ AnimKey::BossSpitStart,		 TEXT("Animations/Ten_Piedad/piedad_spit_start_anim.json"),		 false },
		{ AnimKey::BossSpitLoop,		 TEXT("Animations/Ten_Piedad/piedad_spit_loop_anim.json"),		 false },
		{ AnimKey::BossSpitToIdle,		 TEXT("Animations/Ten_Piedad/piedad_spit_backToIdle.json"),		 false },
		{ AnimKey::BossStomp,			 TEXT("Animations/Ten_Piedad/piedad_stomp_anim.json"),			 false },
		{ AnimKey::BossGroundSmash,		 TEXT("Animations/Ten_Piedad/piedad_ground_smash_anim.json"),	 false },
		{ AnimKey::BossGroundSmashToIdle,TEXT("Animations/Ten_Piedad/piedad_ground_smashToIdle_anim.json"),false },

		// Death
		{ AnimKey::Dead, TEXT("Animations/Ten_Piedad/piedad_death.json"), false },
	};
	return animations;
}
