#include "pch.h"
#include "CCharacter.h"

#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Util/AnimEventHelper.h"

CCharacter::CCharacter() : animator(nullptr), stateSystem(nullptr), abilitySystem(nullptr)
{
    scale = Vec2(100	, 100);
}

CCharacter::~CCharacter()
{
}

wstring CCharacter::GetRandomBloodVfxKey() const
{
    return TEXT("VFX_Blood1");
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
}

void CCharacter::OnCollisionStay(CCollider* other)
{
}

void CCharacter::OnCollisionExit(CCollider* other)
{
}

void CCharacter::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
    assert(animator);
    CAnimation* animation = LOADANIMATION(name+L"_"+aniName, path);
    assert(animation);
    animation->SetRepeat(bShouldRepeat);
    animator->AddAnimation(aniName, animation);
}
