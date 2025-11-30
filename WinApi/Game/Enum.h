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
	Tag_None				= 0,
	Tag_Grounded			= 1 << 0,	// 지상
	Tag_Airborne			= 1 << 1,	// 공중
	Tag_Attacking			= 1 << 2,	// 공격 중
	Tag_Moving				= 1 << 3,	// 이동 중
	Tag_SpecialAction		= 1 << 4,
	Tag_Invincible			= 1 << 5,	// 무적
	Tag_Stunned				= 1 << 6,	// 스턴
	Tag_CanCombo			= 1 << 7,	// 콤보 가능
	Tag_Hit					= 1 << 8,	// 피격
	Tag_Sliding				= 1 << 9,	// 슬라이딩
	Tag_Parrying			= 1 << 10,	// 패리 중
	Tag_Jumping				= 1 << 11,	// 점프 중
	Tag_AbilityAnimation	= 1 << 12,	// Ability가 애니메이션 제어 중
	Tag_BlockMovement		= 1 << 13,
	Tag_StopVelocity 		= 1 << 14,
	Tag_Crouching			= 1 << 15,
	Tag_AirAttackExhausted	= 1 << 16,	// 공중 공격 소진
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

// StateTag 헬퍼 함수
inline bool HasTag(StateTag tags, StateTag check)
{
	return (tags & check) == check;
}

inline bool HasAnyTag(StateTag tags, StateTag check)
{
	return (tags & check) != Tag_None;
}

inline bool TagAdded(StateTag oldTags, StateTag newTags, StateTag tag)
{
	return !HasTag(oldTags, tag) && HasTag(newTags, tag);
}

inline bool TagRemoved(StateTag oldTags, StateTag newTags, StateTag tag)
{
	return HasTag(oldTags, tag) && !HasTag(newTags, tag);
}

//========================================
//##			Ability 이름			##
//========================================

enum class EAbility
{
	None,
	Attack,
	AirAttack,
	CrouchAttack,
	Jump,
	Slide,
	Crouch,
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
	ParryWindowOpen,
	ParryWindowClose,
	Recover,
	// 게임플레이 이벤트
	Hit,
	EndCrouch,
	// Input
	Input_Attack_Pressed,
	Input_Crouch_Released,
};

//========================================
//##			Damage Types			##
//========================================

enum class EDamageType
{
	None,
	Slash,
};

//========================================
//##			Overlay UI Types		##
//========================================

enum class EOverlayUI
{
	None,
	Inventory,
};