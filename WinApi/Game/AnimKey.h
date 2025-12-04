#pragma once

namespace AnimKey
{
	// Common
	constexpr const wchar_t* Idle			= L"Idle";
	constexpr const wchar_t* Hit			= L"Hit";
	constexpr const wchar_t* ParryHit			= L"ParryHit";
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
	
	// Monster
	constexpr const wchar_t* Attack			= L"Attack";
}
