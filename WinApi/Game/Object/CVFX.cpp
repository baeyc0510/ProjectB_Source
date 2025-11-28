#include "pch.h"
#include "CVFX.h"

CVFX::CVFX() : animator(nullptr), lifetime(0), remainingLifetime(0), bHasLifetime(false), bIsPlaying(false),
               bLooping(false)
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
    if (bIsPlaying && bHasLifetime)
    {
        remainingLifetime -= DT;
        remainingLifetime = max(remainingLifetime, 0.0f);
    }
    
    if (bHasLifetime && IsNearlyEqual(remainingLifetime, 0.0f))
    {
        EVENT->Delete(GetScene(), this);
        return;
    }
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

void CVFX::SetLifetime(float inLifetime)
{
    lifetime = inLifetime;
    bHasLifetime = true;
}

float CVFX::GetRemainingLifetime() const
{
    return remainingLifetime;
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
    remainingLifetime = lifetime;
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
    if (!bHasLifetime)
    {
        EVENT->Delete(GetScene(), this);
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
