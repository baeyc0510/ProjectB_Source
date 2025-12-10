#include "pch.h"
#include "CCharacter.h"

#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CStatComponent.h"
#include "Game/Component/CCharacterMovement.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"
#include "Game/Util/AnimEventHelper.h"

CCharacter::CCharacter()
{
    scale = Vec2(100, 100);
}

CCharacter::~CCharacter()
{
}

wstring CCharacter::GetRandomBloodVfxKey() const
{
    return VFXKey::Blood1;
}

void CCharacter::SetIgnorePlatform(UINT platformID)
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

void CCharacter::SetIsGrounded(bool grounded)
{
    if (movement)
    {
        movement->SetGrounded(grounded);
    }
}

bool CCharacter::IsGrounded() const
{
    if (movement)
        return movement->IsGrounded();
    return false;
}

UINT CCharacter::GetCurrentGroundID() const
{
    return movement ? movement->GetActiveGroundID() : 0;
}

float CCharacter::GetPlatformMinX() const
{
    return movement ? movement->GetGroundMinX() : -FLT_MAX;
}

float CCharacter::GetPlatformMaxX() const
{
    return movement ? movement->GetGroundMaxX() : FLT_MAX;
}

bool CCharacter::HasPlatformBounds() const
{
    return movement ? movement->HasGroundBounds() : false;
}

void CCharacter::Init()
{
    // Rigidbody
    rigidbody = new CRigidbody();
    AddChild(rigidbody);

    // Collider
    collider = new CBoxCollider();
    AddChild(collider);

    // MovementComponent
    movement = new CCharacterMovement();
    AddChild(movement);

    // StateSystem
    stateSystem = new CStateSystem();
    stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags)
    {
        OnStateChanged(oldTags, newTags);
    });
    AddChild(stateSystem);

    // StatComponent
    statComponent = new CStatComponent();
    AddChild(statComponent);

    // AbilitySystem
    abilitySystem = new CAbilitySystem();
    AddChild(abilitySystem);

    // Animator
    animator = new CAnimator();
    AddChild(animator);

    // Animator -> AbilitySystem 이벤트 연결
    AnimEventHelper::ConnectAbilitySystem(animator, abilitySystem);

    // Animator -> Character 이벤트 연결
    animator->OnFrameEvent.Add([this](const wstring& eventName) {
        HandleAnimationEvent(AnimEventHelper::ToGameEvent(eventName));
    });
}

void CCharacter::OnEnable()
{
    bWasOnSteepSlope = false;
}

void CCharacter::Update()
{
	// 속도 정지 태그 처리
	if (stateSystem && stateSystem->HasTag(Tag_StopVelocity))
	{
		if (rigidbody)
			rigidbody->SetVelocity(Vec2(0.0f, 0.0f));
	}
}

void CCharacter::Render()
{
}

void CCharacter::OnDisable()
{
}

void CCharacter::Release()
{
}

void CCharacter::OnCollisionEnter(CCollider* other)
{
    if (movement)
    {
        movement->HandleCollisionEnter(other);
    }
}

void CCharacter::OnCollisionStay(CCollider* other)
{
    if (movement)
    {
        movement->HandleCollisionStay(other);
    }
}

void CCharacter::OnCollisionExit(CCollider* other)
{
    if (movement)
    {
        movement->HandleCollisionExit(other);
    }
}

void CCharacter::UpdateStates()
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

void CCharacter::OnStateChanged(EStateTag oldTags, EStateTag newTags)
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

void CCharacter::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
    assert(animator);
    CAnimation* animation = LOADANIMATION(name + L"_" + aniName, path);
    assert(animation);
    animation->SetRepeat(bShouldRepeat);
    animator->AddAnimation(aniName, animation);
}

void CCharacter::SpawnDamageVFX(const CombatContext& context, int spawnDirection)
{
    Vec2 spawnPos = context.hitResult.hitCenter;

    // Hit VFX
    if (!context.vfxKey.empty())
    {
        if (CVFX* vfx = VFX->CreateVFX(context.vfxKey, spawnPos, spawnDirection))
        {
            vfx->PlayVFX();
        }
    }

    // Blood VFX (only if actual damage)
    if (context.value > 0.0001f)
    {
        if (CVFX* vfx = VFX->CreateVFX(GetRandomBloodVfxKey(), spawnPos, spawnDirection))
        {
            vfx->PlayVFX();
        }
    }
}
