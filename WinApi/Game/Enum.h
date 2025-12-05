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
	Stage_Boss01,
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
	Ladder,
	Platform,
	Transition,
	Ledge,		// (플랫폼 가장자리)
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
	Tag_ShouldCrouch		= 1 << 16,
	Tag_AirAttackExhausted	= 1 << 17,	// 공중 공격 소진
	Tag_FlaskRemaining		= 1 << 18,	// 잔여 플라스크 있음
	Tag_Climbing			= 1 << 19,	// 사다리 타는 중
	Tag_CanClimb			= 1 << 20,	// 사다리 진입 가능
	Tag_HasTarget			= 1 << 21,	// AI: 타겟 보유
	Tag_AIPatrol			= 1 << 22,	// AI: 순찰 중
	Tag_AIChase				= 1 << 23,	// AI: 추격 중
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
	Climb,
	Parry,
	CounterAttack,
	Hit,
	ParryHit,
	Death,
	// AI
	AI_Patrol,
	AI_Chase,
	AI_Attack,
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
	Input_Jump_Pressed,
	Input_Attack_Pressed,
	Input_Crouch_Released,
	Input_Up_Hold,
	Input_Up_Released,
	Input_Down_Hold,
	Input_Down_Released,
	// AI 이벤트
	AI_TargetDetected,		// 타겟 감지
	AI_TargetLost,			// 타겟 놓침
	AI_TargetInAttackRange,	// 공격 범위 진입
	AI_PatrolPointReached,	// 순찰 지점 도달
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