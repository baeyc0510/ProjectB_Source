#pragma once

namespace SFXKey
{
    // 플레이어 - 공격
    constexpr const wchar_t* PlayerHeavySlash = L"PlayerHeavySlash";
    constexpr const wchar_t* PlayerLungeAttack = L"PlayerLungeAttack";
    constexpr const wchar_t* PlayerSlashAir1 = L"PlayerSlashAir1";
    constexpr const wchar_t* PlayerSlashAir2 = L"PlayerSlashAir2";
    constexpr const wchar_t* PlayerSlashAir3 = L"PlayerSlashAir3";
    constexpr const wchar_t* PlayerSlashAir4 = L"PlayerSlashAir4";

    // 플레이어 - 적 타격
    constexpr const wchar_t* PlayerEnemyHit1 = L"PlayerEnemyHit1";
    constexpr const wchar_t* PlayerEnemyHit2 = L"PlayerEnemyHit2";
    constexpr const wchar_t* PlayerHeavyEnemyHit = L"PlayerHeavyEnemyHit";

    // 플레이어 - 피격
    constexpr const wchar_t* PlayerDamage = L"PlayerDamage";
    constexpr const wchar_t* PlayerHeavyDamage = L"PlayerHeavyDamage";
    constexpr const wchar_t* PlayerPushback = L"PlayerPushback";

    // 플레이어 - 사망
    constexpr const wchar_t* PlayerDeath = L"PlayerDeath";
    constexpr const wchar_t* PlayerSpikesDeath = L"PlayerSpikesDeath";
    constexpr const wchar_t* PlayerBossDeathHit = L"PlayerBossDeathHit";

    // 플레이어 - 이동
    constexpr const wchar_t* PlayerJump = L"PlayerJump";
    constexpr const wchar_t* PlayerSlide = L"PlayerSlide";
    constexpr const wchar_t* PlayerRun1 = L"PlayerRun1";
    constexpr const wchar_t* PlayerRun2 = L"PlayerRun2";
    constexpr const wchar_t* PlayerClimbLadder = L"PlayerClimbLadder";
    constexpr const wchar_t* PlayerLedgeGrab = L"PlayerLedgeGrab";

    // 플레이어 - 패리
    constexpr const wchar_t* PlayerStartParry = L"PlayerStartParry";
    constexpr const wchar_t* PlayerParryCounterHit = L"PlayerParryHit";
    constexpr const wchar_t* PlayerParrySuccess = L"PlayerParrySlow";
    constexpr const wchar_t* PlayerGuard = L"PlayerGuard";

    // 플레이어 - 기타
    constexpr const wchar_t* PlayerHealing = L"PlayerHealing";
    constexpr const wchar_t* PlayerRespawn = L"PlayerRespawn";
    constexpr const wchar_t* PlayerGetItem = L"PlayerGetItem";
    constexpr const wchar_t* PlayerOverthrow = L"PlayerOverthrow";
    
    // Acolyte
    constexpr const wchar_t* AcolyteDeath = L"AcolyteDeath";
    constexpr const wchar_t* AcolyteStep1 = L"AcolyteStep1";
    constexpr const wchar_t* AcolyteStep2 = L"AcolyteStep2";
    constexpr const wchar_t* AcolytePrepareAttack = L"AcolytePrepareAttack";
    constexpr const wchar_t* AcolyteReleaseAttack = L"AcolyteReleaseAttack";
    
    
    // 보스 - Piedad
    constexpr const wchar_t* PiedadSlash = L"PiedadSlash";
    constexpr const wchar_t* PiedadSmash = L"PiedadSmash";
    constexpr const wchar_t* PiedadSmashVoice = L"PiedadSmashVoice";
    constexpr const wchar_t* PiedadSmashGetUp = L"PiedadSmashGetUp";
    constexpr const wchar_t* PiedadSmashGetUpVoice = L"PiedadSmashGetUpVoice";
    constexpr const wchar_t* PiedadSpitVoice = L"PiedadSpitVoice";
    constexpr const wchar_t* PiedadStomp = L"PiedadStomp";
    constexpr const wchar_t* PiedadTurn = L"PiedadTurn";
    constexpr const wchar_t* PiedadWakeUp = L"PiedadWakeUp";
    
    
    // BGM
    constexpr const wchar_t* BGM_Piedad = L"BGM_Piedad"; // 보스
    constexpr const wchar_t* BGM_Title  = L"BGM_Title"; // 타이틀
    
    
    // 기타
    constexpr const wchar_t* MapEnter = L"BGM_RoomEnter";
}
