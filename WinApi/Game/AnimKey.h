#pragma once

namespace AnimKey
{
	// Common
	constexpr const wchar_t* Idle			= L"Idle";
	constexpr const wchar_t* Hit			= L"Hit";
	constexpr const wchar_t* ParryHit		= L"ParryHit";
	constexpr const wchar_t* Walk			= L"Walk";
	

	// Player Movement
	constexpr const wchar_t* Run				= L"Run";
	constexpr const wchar_t* JumpStart_Inplace	= L"JumpStart_Inplace";
	constexpr const wchar_t* JumpStart_Moving	= L"JumpStart_Moving";
	constexpr const wchar_t* Fall_Inplace		= L"Fall_Inplace";
	constexpr const wchar_t* Fall_Moving		= L"Fall_Moving";
	constexpr const wchar_t* Landed_Inplace		= L"Landed_Inplace";
	constexpr const wchar_t* Landed_Moving		= L"Landed_Moving";
	
	constexpr const wchar_t* Slide			= L"Slide";
	constexpr const wchar_t* Crouch			= L"Crouch";
	constexpr const wchar_t* CrouchUp		= L"CrouchUp";
	constexpr const wchar_t* Climbing		= L"Climbing";
	constexpr const wchar_t* LedgeHang		= L"LedgeHang";
	constexpr const wchar_t* LedgeClimbOver	= L"LedgeClimbOver";
	
	constexpr const wchar_t* Pushback		= L"Pushback";
	constexpr const wchar_t* Pushback_Land	= L"Pushback_Land";

	// Player Attack
	constexpr const wchar_t* Combo1			= L"Combo1";
	constexpr const wchar_t* Combo2			= L"Combo2";
	constexpr const wchar_t* Combo3			= L"Combo3";
	constexpr const wchar_t* AirCombo1		= L"AirCombo1";
	constexpr const wchar_t* AirCombo2		= L"AirCombo2";
	constexpr const wchar_t* CrouchAttack	= L"CrouchAttack";
	
	constexpr const wchar_t* Parry			= L"Parry";
	constexpr const wchar_t* ParrySuccess	= L"ParrySuccess";
	constexpr const wchar_t* ParryCounter	= L"ParryCounter";

	// Player Special Action
	constexpr const wchar_t* UseFlask		= L"UseFlask";
	constexpr const wchar_t* Rising			= L"Rising";
	
	// Monster
	constexpr const wchar_t* Attack			= L"Attack";

	// Boss - Ten Piedad
	constexpr const wchar_t* BossAppear				= L"BossAppear";
	constexpr const wchar_t* Turnaround				= L"Turnaround";
	constexpr const wchar_t* WalkToIdle				= L"WalkToIdle";
	constexpr const wchar_t* BossSlash				= L"BossSlash";
	constexpr const wchar_t* BossSpitStart			= L"BossSpitStart";
	constexpr const wchar_t* BossSpitLoop			= L"BossSpitLoop";
	constexpr const wchar_t* BossSpitToIdle			= L"BossSpitToIdle";
	constexpr const wchar_t* BossStomp				= L"BossStomp";
	constexpr const wchar_t* BossGroundSmash		= L"BossGroundSmash";
	constexpr const wchar_t* BossGroundSmashToIdle	= L"BossGroundSmashToIdle";
	
	// Projectile
	constexpr const wchar_t* ThornBall	= L"ThornBall";
	constexpr const wchar_t* ThornBallDestroyed	= L"ThornBallDestroyed";
	constexpr const wchar_t* ThornGrowth	= L"ThornGrowth";
	constexpr const wchar_t* ThornGrowthDestroyed	= L"ThornGrowthDestroyed";
	constexpr const wchar_t* ThornThrust = L"ThornThrust";
}
