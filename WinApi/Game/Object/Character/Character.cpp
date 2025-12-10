#include "pch.h"
#include "Character.h"

#include "Game/VFXKeys.h"
#include "Game/Component/AbilitySystem.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Component/StateSystem.h"
#include "Game/Component/StatComponent.h"
#include "Game/Component/CharacterMovement.h"
#include "Game/Manager/VFXManager.h"
#include "Game/Object/VFXObject.h"
#include "Game/Util/AnimEventHelper.h"

Character::Character()
{
    scale = Vec2(100, 100);
}

Character::~Character()
{
}

wstring Character::GetRandomBloodVfxKey() const
{
    return VFXKey::Blood1;
}

void Character::SetIgnorePlatform(UINT platformID)
{
    if (movement)
    {
        if (platformID == GetCurrentGroundID())
        {
            movement->SetGrounded(false);
        }
        movement->SetIgnorePlatform(platformID);
    }
}

void Character::SetIsGrounded(bool grounded)
{
    if (movement)
    {
        movement->SetGrounded(grounded);
    }
}

bool Character::IsGrounded() const
{
    if (movement)
        return movement->IsGrounded();
    return false;
}

UINT Character::GetCurrentGroundID() const
{
    return movement ? movement->GetActiveGroundID() : 0;
}

float Character::GetPlatformMinX() const
{
    return movement ? movement->GetGroundMinX() : -FLT_MAX;
}

float Character::GetPlatformMaxX() const
{
    return movement ? movement->GetGroundMaxX() : FLT_MAX;
}

bool Character::HasPlatformBounds() const
{
    return movement ? movement->HasGroundBounds() : false;
}

void Character::StopHorizontalMovement()
{
    if (rigidbody)
        rigidbody->SetVelocity(Vec2(0.f, rigidbody->GetVelocity().y));
}

void Character::Init()
{
    // Rigidbody
    rigidbody = new Rigidbody();
    AddChild(rigidbody);

    // Collider
    collider = new BoxCollider();
    AddChild(collider);

    // MovementComponent
    movement = new CharacterMovement();
    AddChild(movement);

    // StateSystem
    stateSystem = new StateSystem();
    stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags)
    {
        OnStateChanged(oldTags, newTags);
    });
    AddChild(stateSystem);

    // StatComponent
    statComponent = new StatComponent();
    // StatComponent 이벤트 바인딩
    statComponent->OnStatChanged.Add([this](EStatType type, float current, float max) {
        OnStatChanged(type, current, max);
    });
    AddChild(statComponent);

    // AbilitySystem
    abilitySystem = new AbilitySystem();
    AddChild(abilitySystem);

    // Animator
    animator = new Animator();
    AddChild(animator);

    // Animator -> AbilitySystem 이벤트 연결
    AnimEventHelper::ConnectAbilitySystem(animator, abilitySystem);

    // Animator -> Character 이벤트 연결
    animator->OnFrameEvent.Add([this](const wstring& eventName) {
        HandleAnimationEvent(AnimEventHelper::ToGameEvent(eventName));
    });
}

void Character::OnEnable()
{
    bWasOnSteepSlope = false;
}

void Character::Update()
{
	// 속도 정지 태그 처리
	if (stateSystem && stateSystem->HasTag(Tag_StopVelocity))
	{
		if (rigidbody)
			rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
	}
}

void Character::Render()
{
}

void Character::OnDisable()
{
}

void Character::Release()
{
}

void Character::OnCollisionEnter(Collider* other)
{
    if (movement)
    {
        movement->HandleCollisionEnter(other);
    }
}

void Character::OnCollisionStay(Collider* other)
{
    if (movement)
    {
        movement->HandleCollisionStay(other);
    }
}

void Character::OnCollisionExit(Collider* other)
{
    if (movement)
    {
        movement->HandleCollisionExit(other);
    }
}

void Character::UpdateStates()
{
    if (!movement)
        return;

    bool bIsGrounded = movement->IsGrounded();
    bool bIsOnSteepSlope = movement->IsOnSteepSlope();

    // 착지 상태
    if (bIsGrounded)
    {
        stateSystem->RemoveTag(Tag_Airborne);
        stateSystem->AddTagUnique(Tag_Grounded);
    }
    else
    {
        stateSystem->RemoveTag(Tag_Grounded);
        stateSystem->AddTagUnique(Tag_Airborne);
    }

    // 가파른 경사면 미끄러짐 (변화 시에만)
    if (bIsOnSteepSlope && !bWasOnSteepSlope)
    {
        stateSystem->AddTag(Tag_BlockMovement);
    }
    else if (!bIsOnSteepSlope && bWasOnSteepSlope)
    {
        stateSystem->RemoveTag(Tag_BlockMovement);
    }
    bWasOnSteepSlope = bIsOnSteepSlope;

    // 끼임 상태
    bool bIsSquashed = movement->IsBeingSquashed();
    if (bIsSquashed)
    {
        stateSystem->AddTagUnique(Tag_Squashed);
    }
    else
    {
        stateSystem->RemoveTag(Tag_Squashed);
    }
}

void Character::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
    // 착지
    if (TagAdded(oldTags, newTags, Tag_Grounded))
    {
        abilitySystem->TriggerEvent(EGameEvent::Landed);
        if (!ShouldIgnorePlatform())
        {
            movement->ClearIgnorePlatform();
        }
    }
}

void Character::OnStatChanged(EStatType type, float& current, float& max)
{
    if (type == EStatType::HP)
    {
        if (IsNearlyEqual(current,0))
        {
            abilitySystem->TryActivateAbility(EAbility::Die);
        }
    }
}

void Character::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
    assert(animator);
    AnimationResource* animation = LOADANIMATION(name + L"_" + aniName, path);
    assert(animation);
    animation->SetRepeat(bShouldRepeat);
    animator->AddAnimation(aniName, animation);
}

void Character::SpawnDamageVFX(const CombatContext& context, int spawnDirection)
{
    Vec2 spawnPos = context.hitResult.hitCenter;

    // Hit VFX
    if (!context.vfxKey.empty())
    {
        if (VFXObject* vfx = VFX->CreateVFX(context.vfxKey, spawnPos, spawnDirection))
        {
            vfx->PlayVFX();
        }
    }

    // Blood VFX (only if actual damage)
    if (context.value > 0.0001f)
    {
        if (VFXObject* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
        {
            vfx->PlayVFX();
        }
    }
}
