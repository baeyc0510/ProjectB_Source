#include "pch.h"
#include "CSFXManager.h"
#include "Game/SFXKeys.h"

void CSFXManager::PreLoad()
{
    // 플레이어 - 공격
    LOADSOUND(SFXKey::PlayerHeavySlash, TEXT("Sound/penitent/PENITENT_HEAVY_SLASH.wav"));
    LOADSOUND(SFXKey::PlayerLungeAttack, TEXT("Sound/penitent/LUNGE_ATTACK_LV3.wav"));
    LOADSOUND(SFXKey::PlayerSlashAir1, TEXT("Sound/penitent/PENITENT_SLASH_AIR_1.wav"));
    LOADSOUND(SFXKey::PlayerSlashAir2, TEXT("Sound/penitent/PENITENT_SLASH_AIR_2.wav"));
    LOADSOUND(SFXKey::PlayerSlashAir3, TEXT("Sound/penitent/PENITENT_SLASH_AIR_3.wav"));
    LOADSOUND(SFXKey::PlayerSlashAir4, TEXT("Sound/penitent/PENITENT_SLASH_AIR_4.wav"));

    // 플레이어 - 적 타격
    LOADSOUND(SFXKey::PlayerEnemyHit1, TEXT("Sound/penitent/PENITENT_ENEMY_HIT_3.wav"));
    LOADSOUND(SFXKey::PlayerEnemyHit2, TEXT("Sound/penitent/PENITENT_ENEMY_HIT_4.wav"));
    LOADSOUND(SFXKey::PlayerHeavyEnemyHit, TEXT("Sound/penitent/PENITENT_HEAVY_ENEMY_HIT.wav"));

    // 플레이어 - 피격
    LOADSOUND(SFXKey::PlayerDamage, TEXT("Sound/penitent/PENITENT_SIMPLE_DAMAGE_DEFAULT.wav"));
    LOADSOUND(SFXKey::PlayerHeavyDamage, TEXT("Sound/penitent/PENITENT_HEAVY_DAMAGE.wav"));
    LOADSOUND(SFXKey::PlayerPushback, TEXT("Sound/penitent/PENITENT_PUSHBACK.wav"));

    // 플레이어 - 사망
    LOADSOUND(SFXKey::PlayerDeath, TEXT("Sound/penitent/PENITENT_DEATH_DEFAULT.wav"));
    LOADSOUND(SFXKey::PlayerSpikesDeath, TEXT("Sound/penitent/PENITENT_SPIKES_DEATH.wav"));
    LOADSOUND(SFXKey::PlayerBossDeathHit, TEXT("Sound/penitent/PENITENT_BOSS_DEATH_HIT.wav"));

    // 플레이어 - 이동
    LOADSOUND(SFXKey::PlayerJump, TEXT("Sound/penitent/PENITENT_JUMP.wav"));
    LOADSOUND(SFXKey::PlayerSlide, TEXT("Sound/penitent/PENITENT_DASH.wav"));
    LOADSOUND(SFXKey::PlayerRun1, TEXT("Sound/penitent/PENITENT_RUN_MARBLE_6.wav"));
    LOADSOUND(SFXKey::PlayerRun2, TEXT("Sound/penitent/PENITENT_RUN_MARBLE_8.wav"));
    LOADSOUND(SFXKey::PlayerClimbLadder, TEXT("Sound/penitent/PENITENT_CLIMB_LADDER_3.wav"));
    LOADSOUND(SFXKey::PlayerLedgeGrab, TEXT("Sound/penitent/Penitent_EdgeGrab.wav"));

    // 플레이어 - 패리
    LOADSOUND(SFXKey::PlayerStartParry, TEXT("Sound/penitent/PENITENT_START_PARRY.wav"));
    LOADSOUND(SFXKey::PlayerParryCounterHit, TEXT("Sound/penitent/PENITENT_PARRY_COUNTER_HIT.wav"));
    LOADSOUND(SFXKey::PlayerParrySuccess, TEXT("Sound/penitent/PENITENT_PARRY_SUCCESS.wav"));
    LOADSOUND(SFXKey::PlayerGuard, TEXT("Sound/penitent/PENITENT_GUARD.wav"));

    // 플레이어 - 기타
    LOADSOUND(SFXKey::PlayerHealing, TEXT("Sound/penitent/HEALING.wav"));
    LOADSOUND(SFXKey::PlayerRespawn, TEXT("Sound/penitent/PENITENT_RESPAWN.wav"));
    LOADSOUND(SFXKey::PlayerGetItem, TEXT("Sound/penitent/GET_FLOOR_ITEM.wav"));
    LOADSOUND(SFXKey::PlayerOverthrow, TEXT("Sound/penitent/PENITENT_OVERTHROW_DEFAULT.wav"));
    
    // Acorite
    LOADSOUND(SFXKey::AcolytePrepareAttack, TEXT("Sound/enemy/ACOLYTE_CHARGE_ATTACK_DEFAULT.wav"));
    LOADSOUND(SFXKey::AcolyteReleaseAttack, TEXT("Sound/enemy/ACOLYTE_RELEASE_ATTACK_DEFAULT.wav"));
    LOADSOUND(SFXKey::AcolyteDeath, TEXT("Sound/enemy/ACOLYTE_DEATH_DEFAULT.wav"));
    LOADSOUND(SFXKey::AcolyteStep1, TEXT("Sound/enemy/ACOLYTE_FOOTSTEPS_DEFAULT_1.wav"));
    LOADSOUND(SFXKey::AcolyteStep2, TEXT("Sound/enemy/ACOLYTE_FOOTSTEPS_DEFAULT_2.wav"));
    
    // // 보스 - Piedad
    LOADSOUND(SFXKey::PiedadSlash, TEXT("Sound/ten_piedad/PIEDAD_SLASH.wav"));
    LOADSOUND(SFXKey::PiedadSmash, TEXT("Sound/ten_piedad/PIEDAD_Smash.wav"));
    LOADSOUND(SFXKey::PiedadSmashVoice, TEXT("Sound/ten_piedad/PIEDAD_SMASH_VOICE.wav"));
    LOADSOUND(SFXKey::PiedadSmashGetUp, TEXT("Sound/ten_piedad/PIEDAD_SmashGetUp.wav"));
    LOADSOUND(SFXKey::PiedadSmashGetUpVoice, TEXT("Sound/ten_piedad/PIEDAD_SMASH_GET_UP_VOICE.wav"));
    LOADSOUND(SFXKey::PiedadSpitVoice, TEXT("Sound/ten_piedad/PIEDAD_SPIT_VOICE.wav"));
    LOADSOUND(SFXKey::PiedadStomp, TEXT("Sound/ten_piedad/PIEDAD_STOMP.wav"));
    LOADSOUND(SFXKey::PiedadTurn, TEXT("Sound/ten_piedad/PIEDAD_TURN.wav"));
    LOADSOUND(SFXKey::PiedadWakeUp, TEXT("Sound/ten_piedad/PIEDAD_WakeUp.wav"));
    
    // BGM
    LOADSOUND(SFXKey::BGM_Piedad, TEXT("Sound/ten_piedad/PIEDAD_MASTER.wav"));
    LOADSOUND(SFXKey::BGM_Title, TEXT("Sound/TITLE.wav"));
    LOADSOUND(SFXKey::MapEnter, TEXT("Sound/ZONE_INFO.wav"));
}

void CSFXManager::PlayOnce(const wstring& key, float volume)
{
    CSound* sound = GetSound(key);
    if (sound)
    {
        SOUND->PlayOnce(sound, volume);
    }
}

void CSFXManager::PlayLoop(const wstring& key, float volume)
{
    CSound* sound = GetSound(key);
    if (sound)
    {
        SOUND->PlayLoop(key, sound, volume);
    }
}

void CSFXManager::Stop(const wstring& key)
{
    SOUND->Stop(key);
}

void CSFXManager::StopAll()
{
    SOUND->StopAll();
}

void CSFXManager::PlayBGM(const wstring& key, float volume)
{
    if (currentBGM == key)
        return;
    
    Stop(currentBGM);
    currentBGM = key;
    PlayLoop(key, volume);
}

CSound* CSFXManager::GetSound(const wstring& key)
{
    return SINGLE(CResourceManager)->SoundFind(key);
}
