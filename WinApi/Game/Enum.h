#pragma once

//========================================
//##				게임씬				##
//========================================

enum SceneType
{
	Title,
	Stage01,

	SceneSize,
};

//========================================
//##				레이어				##
//========================================

enum Layer
{
	Default,
	Player,
	Monster,
	Missile,
	Ground,

	LayerSize,
};

//========================================
//##			StateTag (비트필드)		##
//========================================

enum StateTag
{
	Tag_None			= 0,
	Tag_Grounded		= 1 << 0,	// 지상
	Tag_Airborne		= 1 << 1,	// 공중
	Tag_Attacking		= 1 << 2,	// 공격 중
	Tag_Moving			= 1 << 3,	// 이동 중
	Tag_Invincible		= 1 << 4,	// 무적
	Tag_Stunned			= 1 << 5,	// 스턴
	Tag_CanCombo		= 1 << 6,	// 콤보 가능
	Tag_Hit				= 1 << 7,	// 피격
	Tag_Sliding			= 1 << 8,	// 슬라이딩
	Tag_Parrying		= 1 << 9,	// 패리 중
	Tag_Jumping			= 1 << 10,	// 점프 중
	Tag_AbilityPlaying	= 1 << 11,	// Ability가 애니메이션 제어 중
};

inline StateTag operator|(StateTag a, StateTag b)
{
	return static_cast<StateTag>(static_cast<int>(a) | static_cast<int>(b));
}

inline StateTag operator&(StateTag a, StateTag b)
{
	return static_cast<StateTag>(static_cast<int>(a) & static_cast<int>(b));
}

inline StateTag operator~(StateTag a)
{
	return static_cast<StateTag>(~static_cast<int>(a));
}

//========================================
//##			Ability 이름			##
//========================================

enum class EAbility
{
	None,
	Attack1,
	Attack2,
	Attack3,
	AirAttack,
	DownAttack,
	Jump,
	Slide,
	Parry,
	CounterAttack,
	Hit,
	Death,
};

//========================================
//##			Game 이벤트				##
//========================================

enum class EGameEvent
{
	None,
	// 애니메이션 이벤트
	HitCheck,
	ComboWindowOpen,
	ComboWindowClose,
	InvincibleStart,
	InvincibleEnd,
	Landed,
	// 게임플레이 이벤트 (추후 확장)
};