#include "pch.h"
#include "SFXData.h"
#include "Game/SFXKeys.h"

const std::vector<SoundEntry>& SFXData::GetAllSounds()
{
	static const std::vector<SoundEntry> sounds = {
		/*~ Player - Attack ~*/
		{ SFXKey::PlayerHeavySlash,		TEXT("Sound/penitent/PENITENT_HEAVY_SLASH.wav") },
		{ SFXKey::PlayerLungeAttack,	TEXT("Sound/penitent/LUNGE_ATTACK_LV3.wav") },
		{ SFXKey::PlayerSlashAir1,		TEXT("Sound/penitent/PENITENT_SLASH_AIR_1.wav") },
		{ SFXKey::PlayerSlashAir2,		TEXT("Sound/penitent/PENITENT_SLASH_AIR_2.wav") },
		{ SFXKey::PlayerSlashAir3,		TEXT("Sound/penitent/PENITENT_SLASH_AIR_3.wav") },
		{ SFXKey::PlayerSlashAir4,		TEXT("Sound/penitent/PENITENT_SLASH_AIR_4.wav") },

		/*~ Player - Enemy Hit ~*/
		{ SFXKey::PlayerEnemyHit1,		TEXT("Sound/penitent/PENITENT_ENEMY_HIT_3.wav") },
		{ SFXKey::PlayerEnemyHit2,		TEXT("Sound/penitent/PENITENT_ENEMY_HIT_4.wav") },
		{ SFXKey::PlayerHeavyEnemyHit,	TEXT("Sound/penitent/PENITENT_HEAVY_ENEMY_HIT.wav") },

		/*~ Player - Damage ~*/
		{ SFXKey::PlayerDamage,			TEXT("Sound/penitent/PENITENT_SIMPLE_DAMAGE_DEFAULT.wav") },
		{ SFXKey::PlayerHeavyDamage,	TEXT("Sound/penitent/PENITENT_HEAVY_DAMAGE.wav") },
		{ SFXKey::PlayerPushback,		TEXT("Sound/penitent/PENITENT_PUSHBACK.wav") },

		/*~ Player - Death ~*/
		{ SFXKey::PlayerDeath,			TEXT("Sound/penitent/PENITENT_DEATH_DEFAULT.wav") },
		{ SFXKey::PlayerSpikesDeath,	TEXT("Sound/penitent/PENITENT_SPIKES_DEATH.wav") },
		{ SFXKey::PlayerBossDeathHit,	TEXT("Sound/penitent/PENITENT_BOSS_DEATH_HIT.wav") },

		/*~ Player - Movement ~*/
		{ SFXKey::PlayerJump,			TEXT("Sound/penitent/PENITENT_JUMP.wav") },
		{ SFXKey::PlayerSlide,			TEXT("Sound/penitent/PENITENT_DASH.wav") },
		{ SFXKey::PlayerRun1,			TEXT("Sound/penitent/PENITENT_RUN_MARBLE_6.wav") },
		{ SFXKey::PlayerRun2,			TEXT("Sound/penitent/PENITENT_RUN_MARBLE_8.wav") },
		{ SFXKey::PlayerClimbLadder,	TEXT("Sound/penitent/PENITENT_CLIMB_LADDER_3.wav") },
		{ SFXKey::PlayerLedgeGrab,		TEXT("Sound/penitent/Penitent_EdgeGrab.wav") },

		/*~ Player - Parry ~*/
		{ SFXKey::PlayerStartParry,		TEXT("Sound/penitent/PENITENT_START_PARRY.wav") },
		{ SFXKey::PlayerParryCounterHit,TEXT("Sound/penitent/PENITENT_PARRY_COUNTER_HIT.wav") },
		{ SFXKey::PlayerParrySuccess,	TEXT("Sound/penitent/PENITENT_PARRY_SUCCESS.wav") },
		{ SFXKey::PlayerGuard,			TEXT("Sound/penitent/PENITENT_GUARD.wav") },

		/*~ Player - Misc ~*/
		{ SFXKey::PlayerHealing,		TEXT("Sound/penitent/HEALING.wav") },
		{ SFXKey::PlayerRespawn,		TEXT("Sound/penitent/PENITENT_RESPAWN.wav") },
		{ SFXKey::PlayerGetItem,		TEXT("Sound/penitent/GET_FLOOR_ITEM.wav") },
		{ SFXKey::PlayerOverthrow,		TEXT("Sound/penitent/PENITENT_OVERTHROW_DEFAULT.wav") },

		/*~ Acolyte ~*/
		{ SFXKey::AcolytePrepareAttack,	TEXT("Sound/enemy/ACOLYTE_CHARGE_ATTACK_DEFAULT.wav") },
		{ SFXKey::AcolyteReleaseAttack,	TEXT("Sound/enemy/ACOLYTE_RELEASE_ATTACK_DEFAULT.wav") },
		{ SFXKey::AcolyteDeath,			TEXT("Sound/enemy/ACOLYTE_DEATH_DEFAULT.wav") },
		{ SFXKey::AcolyteStep1,			TEXT("Sound/enemy/ACOLYTE_FOOTSTEPS_DEFAULT_1.wav") },
		{ SFXKey::AcolyteStep2,			TEXT("Sound/enemy/ACOLYTE_FOOTSTEPS_DEFAULT_2.wav") },

		/*~ Boss - Ten Piedad ~*/
		{ SFXKey::PiedadSlash,			TEXT("Sound/ten_piedad/PIEDAD_SLASH.wav") },
		{ SFXKey::PiedadSmash,			TEXT("Sound/ten_piedad/PIEDAD_Smash.wav") },
		{ SFXKey::PiedadSmashVoice,		TEXT("Sound/ten_piedad/PIEDAD_SMASH_VOICE.wav") },
		{ SFXKey::PiedadSmashGetUp,		TEXT("Sound/ten_piedad/PIEDAD_SmashGetUp.wav") },
		{ SFXKey::PiedadSmashGetUpVoice,TEXT("Sound/ten_piedad/PIEDAD_SMASH_GET_UP_VOICE.wav") },
		{ SFXKey::PiedadSpitVoice,		TEXT("Sound/ten_piedad/PIEDAD_SPIT_VOICE.wav") },
		{ SFXKey::PiedadStomp,			TEXT("Sound/ten_piedad/PIEDAD_STOMP.wav") },
		{ SFXKey::PiedadTurn,			TEXT("Sound/ten_piedad/PIEDAD_TURN.wav") },
		{ SFXKey::PiedadWakeUp,			TEXT("Sound/ten_piedad/PIEDAD_WakeUp.wav") },
		{ SFXKey::PiedadDeath,			TEXT("Sound/ten_piedad/PIEDAD_DEATH.wav") },
		{ SFXKey::PiedadDeathVoice,		TEXT("Sound/ten_piedad/PIEDAD_DEATH_VOICE.wav") },

		/*~ BGM & System ~*/
		{ SFXKey::BGM_Piedad,	TEXT("Sound/ten_piedad/PIEDAD_MASTER.wav") },
		{ SFXKey::BGM_Title,	TEXT("Sound/TITLE.wav") },
		{ SFXKey::MapEnter,		TEXT("Sound/ZONE_INFO.wav") },
		{ SFXKey::ClearBoss,	TEXT("Sound/Boss_Clear.wav") },
	};
	return sounds;
}
