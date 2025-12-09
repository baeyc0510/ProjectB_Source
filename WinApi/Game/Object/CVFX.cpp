#include "pch.h"
#include "CVFX.h"

CVFX::CVFX() : animator(nullptr), animation(nullptr), bIsPlaying(false), bLooping(false)
{
}

CVFX::~CVFX()
{
}

void CVFX::Init()
{
    animator = new CAnimator();
    AddChild(animator);
}

void CVFX::OnEnable()
{
}

void CVFX::Update()
{
    // Lifetime 처리는 CGameObject::ComponentUpdate에서 자동으로 수행
}

void CVFX::Render()
{
}

void CVFX::OnDisable()
{
    bIsPlaying = false;
}

void CVFX::Release()
{
}

void CVFX::SetLooping(bool inLooping)
{
    bLooping = inLooping;
    if (animation != nullptr)
    {
        animation->SetRepeat(inLooping);
    }
}

void CVFX::PlayVFX()
{
    bIsPlaying = true;
    animator->Play(TEXT("VFX"), true, BIND(this, OnFinishedAnimation));
    animator->SetDirection(GetForward());
}

void CVFX::StopVFX()
{
    bIsPlaying = false;
    animator->Stop();
}

void CVFX::OnFinishedAnimation()
{
    // Lifetime이 없으면 애니메이션 종료 시 삭제
    if (!HasLifetime())
    {
        Destroy();
    }
}

void CVFX::SetAnimation(CAnimation* inAnimation)
{
    assert(inAnimation);
    animation = inAnimation;
    animation->SetRepeat(bLooping);

    animator->Reset();
    animator->AddAnimation(TEXT("VFX"), animation);
}
