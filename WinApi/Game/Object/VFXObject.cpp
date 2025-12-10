#include "pch.h"
#include "VFXObject.h"

VFXObject::VFXObject() : animator(nullptr), animation(nullptr), bIsPlaying(false), bLooping(false)
{
}

VFXObject::~VFXObject()
{
}

void VFXObject::Init()
{
    animator = new Animator();
    AddChild(animator);
}

void VFXObject::OnEnable()
{
}

void VFXObject::Update()
{
    // Lifetime 처리는 CGameObject::ComponentUpdate에서 자동으로 수행
}

void VFXObject::Render()
{
}

void VFXObject::OnDisable()
{
    bIsPlaying = false;
}

void VFXObject::Release()
{
}

void VFXObject::SetLooping(bool inLooping)
{
    bLooping = inLooping;
    if (animation != nullptr)
    {
        animation->SetRepeat(inLooping);
    }
}

void VFXObject::PlayVFX()
{
    bIsPlaying = true;
    animator->Play(TEXT("VFX"), true, BIND(this, OnFinishedAnimation));
    animator->SetDirection(GetForward());
}

void VFXObject::StopVFX()
{
    bIsPlaying = false;
    animator->Stop();
}

void VFXObject::OnFinishedAnimation()
{
    // Lifetime이 없으면 애니메이션 종료 시 삭제
    if (!HasLifetime())
    {
        Destroy();
    }
}

void VFXObject::SetAnimation(AnimationResource* inAnimation)
{
    assert(inAnimation);
    animation = inAnimation;
    animation->SetRepeat(bLooping);

    animator->Reset();
    animator->AddAnimation(TEXT("VFX"), animation);
}
