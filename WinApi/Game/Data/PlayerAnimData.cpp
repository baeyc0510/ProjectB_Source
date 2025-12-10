#include "pch.h"
#include "PlayerAnimData.h"
#include "Game/AnimKey.h"

const std::vector<AnimationEntry>& PlayerAnimData::GetAnimations()
{
	static const std::vector<AnimationEntry> animations = {
		// Locomotion
		{ AnimKey::Idle,			  TEXT("Animations/Penitent/penitent_idle_anim.json"),			true },
		{ AnimKey::Run,				  TEXT("Animations/Penitent/penitent_running_anim.json"),		true },
		{ AnimKey::JumpStart_Inplace, TEXT("Animations/Penitent/jump_start_inplace.json"),			false },
		{ AnimKey::JumpStart_Moving,  TEXT("Animations/Penitent/jump_start_moving.json"),			false },
		{ AnimKey::Fall_Inplace,	  TEXT("Animations/Penitent/penitent_falling_loop.json"),		true },
		{ AnimKey::Fall_Moving,		  TEXT("Animations/Penitent/falling_moving.json"),				true },
		{ AnimKey::Landed_Inplace,	  TEXT("Animations/Penitent/jump_landed_inplace.json"),			false },
		{ AnimKey::Landed_Moving,	  TEXT("Animations/Penitent/jump_landed_moving.json"),			false },

		// Combat
		{ AnimKey::Combo1,		  TEXT("Animations/Penitent/penitent_attack_combo_1.json"),		false },
		{ AnimKey::Combo2,		  TEXT("Animations/Penitent/penitent_attack_combo_2.json"),		false },
		{ AnimKey::Combo3,		  TEXT("Animations/Penitent/penitent_attack_combo_3.json"),		false },
		{ AnimKey::AirCombo1,	  TEXT("Animations/Penitent/penitent_jumping_attack1.json"),	false },
		{ AnimKey::AirCombo2,	  TEXT("Animations/Penitent/penitent_jumping_attack2.json"),	false },
		{ AnimKey::CrouchAttack,  TEXT("Animations/Penitent/penitent_crouch_attack_anim.json"),	false },
		{ AnimKey::Parry,		  TEXT("Animations/Penitent/penitent_parry.json"),				false },
		{ AnimKey::ParrySuccess,  TEXT("Animations/Penitent/penitent_parry_success.json"),		false },
		{ AnimKey::ParryCounter,  TEXT("Animations/Penitent/penitent_parry_counter.json"),		false },

		// Actions
		{ AnimKey::Slide,		  TEXT("Animations/Penitent/penitent_dodge_anim.json"),				false },
		{ AnimKey::Crouch,		  TEXT("Animations/Penitent/penitent_crouch_anim.json"),			false },
		{ AnimKey::CrouchUp,	  TEXT("Animations/Penitent/penitent_crouch_up_anim.json"),			false },
		{ AnimKey::UseFlask,	  TEXT("Animations/Penitent/penitent_healthposion_anim.json"),		false },
		{ AnimKey::Climbing,	  TEXT("Animations/Penitent/penitent_ladder_climb_loop_anim.json"),	true },
		{ AnimKey::LedgeHang,	  TEXT("Animations/Penitent/penitent_hangonledge_anim.json"),		false },
		{ AnimKey::LedgeClimbOver,TEXT("Animations/Penitent/penitent_climbledge.json"),				false },

		// Hit Reactions
		{ AnimKey::Pushback,	  TEXT("Animations/Penitent/penitent_pushback_anim.json"),		false },
		{ AnimKey::Pushback_Land, TEXT("Animations/Penitent/penitent_pushback_land_anim.json"),	false },
		{ AnimKey::Rising,		  TEXT("Animations/Penitent/player_rising.json"),				false },
		{ AnimKey::Dead,		  TEXT("Animations/Penitent/penitent_death_anim.json"),			false },
	};
	return animations;
}
