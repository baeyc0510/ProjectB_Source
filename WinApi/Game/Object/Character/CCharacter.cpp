#include "pch.h"
#include "CCharacter.h"

#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Util/AnimEventHelper.h"

CCharacter::CCharacter()
    : animator(nullptr)
    , stateSystem(nullptr)
    , abilitySystem(nullptr)
    , rigidbody(nullptr)
    , collider(nullptr)
    , bIsGrounded(false)
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

void CCharacter::Init()
{
    // StateSystem
    stateSystem = new CStateSystem();
    AddChild(stateSystem);

    // AbilitySystem
    abilitySystem = new CAbilitySystem();
    AddChild(abilitySystem);

    // Animator
    animator = new CAnimator();
    AddChild(animator);

    // Animator -> AbilitySystem 이벤트 연결
    AnimEventHelper::ConnectAbilitySystem(animator, abilitySystem);
}

void CCharacter::OnEnable()
{
}

void CCharacter::Update()
{
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
    if (other->GetLayer() == Layer::Ground)
    {
        HandleGroundCollision(other);
    }
}

void CCharacter::OnCollisionStay(CCollider* other)
{
    if (other->GetLayer() == Layer::Ground)
    {
        bIsGrounded = true;
        HandleGroundCollision(other);

        // 아래로 떨어지는 속도 제거
        if (rigidbody)
        {
            Vec2 velocity = rigidbody->GetVelocity();
            if (velocity.y > 0)
            {
                velocity.y = 0.f;
                rigidbody->SetVelocity(velocity);
            }
        }
    }
}

void CCharacter::OnCollisionExit(CCollider* other)
{
    if (other->GetLayer() == Layer::Ground)
    {
        bIsGrounded = false;
    }
}

void CCharacter::UpdateGroundState()
{
    // 착지 체크
    if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
    {
        stateSystem->RemoveTag(Tag_Airborne);
        stateSystem->AddTagUnique(Tag_Grounded);
    }
    if (!bIsGrounded)
    {
        stateSystem->RemoveTag(Tag_Grounded);
        stateSystem->AddTagUnique(Tag_Airborne);
    }
}

void CCharacter::HandleGroundCollision(CCollider* ground)
{
    if (!collider || !rigidbody)
        return;

    // 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
    float characterBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
    float groundTop = ground->GetPos().y - ground->GetScale().y / 2.f;

    float overlap = characterBottom - groundTop;
    if (overlap > 0)
    {
        Vec2 characterPos = GetPos();
        characterPos.y -= overlap;
        SetPos(characterPos);
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
