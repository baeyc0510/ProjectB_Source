#pragma once

//========================================
//##				게임씬				##
//========================================

enum ESceneType
{
	Title,
	Stage01,
	Stage02,
	Stage03,
	Stage04,
};

//========================================
//##				레이어				##
//========================================

enum ELayer
{
	Default,
	Player,
	Monster,
	Ground,
	Transition,
	LayerSize,
};

//========================================
//##			StateTag (비트필드)		##
//========================================

enum EStateTag
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
	Tag_FlaskRemaining		= 1 << 17,	// 잔여 플라스크 있음
	Tag_Climbing			= 1 << 18,	// 사다리 타는 중
	Tag_CanClimb			= 1 << 19,	// 사다리 진입 가능
};

inline EStateTag operator|(EStateTag a, EStateTag b)
{
	return static_cast<EStateTag>(static_cast<int>(a) | static_cast<int>(b));
}

inline EStateTag operator&(EStateTag a, EStateTag b)
{
	return static_cast<EStateTag>(static_cast<int>(a) & static_cast<int>(b));
}

inline EStateTag operator~(EStateTag a)
{
	return static_cast<EStateTag>(~static_cast<int>(a));
}

// StateTag 헬퍼 함수
inline bool HasTag(EStateTag tags, EStateTag check)
{
	return (tags & check) == check;
}

inline bool HasAnyTag(EStateTag tags, EStateTag check)
{
	return (tags & check) != Tag_None;
}

inline bool TagAdded(EStateTag oldTags, EStateTag newTags, EStateTag tag)
{
	return !HasTag(oldTags, tag) && HasTag(newTags, tag);
}

inline bool TagRemoved(EStateTag oldTags, EStateTag newTags, EStateTag tag)
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
	UseFlask,
	Jump,
	Slide,
	Crouch,
	Parry,
	CounterAttack,
	Hit,
	ParryHit,
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
	DoAction,
	// 게임플레이 이벤트
	Hit,
	EndCrouch,
	VelocityChanged,
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
	Parry,
};

//========================================
//##			Overlay UI Types		##
//========================================

enum class EOverlayUI
{
	None,
	Inventory,
};

//========================================
//##			Terrain Types			##
//========================================

enum class ETerrain
{
	Empty,			// 빈 공간 (통과)
	Solid,			// 솔리드 (벽/바닥)
	OneWay,			// 원웨이 플랫폼
	Ladder,			// 사다리
	WallClimb,		// 벽타기
	Ledge,			// 매달리기
	Swamp,			// 늪 (감속)
	Damage,			// 데미지 (가시)
};