#include "pch.h"
#include "Animator.h"

Animator::Animator()
{
}

Animator::~Animator()
{
}

// Animation 관리
void Animator::AddAnimation(const wstring& aniName, AnimationResource* animation)
{
	AnimationResource* ani = FindAnimation(aniName);
	assert(nullptr == ani && "Animation already exist");

	animation->SetKey(aniName);
	animation->SetPath(TEXT(""));
	animationMap.insert(make_pair(aniName, animation));
}

void Animator::RemoveAnimation(const wstring& aniName)
{
	AnimationResource* ani = FindAnimation(aniName);
	if (ani == nullptr)
		return;

	animationMap.erase(aniName);
	if (!ani->IsCached())
		delete ani;
}

AnimationResource* Animator::FindAnimation(const wstring& aniName)
{
	auto iter = animationMap.find(aniName);
	return (iter != animationMap.end()) ? iter->second : nullptr;
}

void Animator::CreateAnimation(const wstring& aniName, ImageResource* image, float stepTime, UINT count, bool repeat, Vec2 pos, Vec2 scale, Vec2 step)
{
	AnimationResource* ani = new AnimationResource();
	ani->Create(image, stepTime, count, repeat, pos, scale, step);
	AddAnimation(aniName, ani);
}

// 재생 제어
void Animator::Play(const wstring& aniName, bool reset,
	Delegate<>::EventFunc onFinishedCallback,
	Delegate<>::EventFunc onInterruptedCallback)
{
	// 동일 애니메이션이 재생 중이고 reset이 아니면 무시
	if (bPlaying && currentAnimation && aniName == currentAnimation->GetKey() && !reset)
		return;

	AnimationResource* animation = FindAnimation(aniName);
	if (!animation)
	{
		assert(animation && "Animation not found");
		return;
	}

	// 이전 애니메이션 중단 처리
	if (currentAnimation && currentAnimation != animation && !bFinished)
	{
		if (onInterrupted.IsBound())
			onInterrupted.Invoke();
	}

	// 프레임 초기화
	if (reset || currentAnimation != animation)
	{
		currentFrame = bReverse ? static_cast<UINT>(animation->frames.size() - 1) : 0;
		currentTime = 0.f;
		bFinished = false;
	}

	// 콜백 등록
	if (onFinishedCallback)
		onFinished.Bind(onFinishedCallback);
	else
		onFinished.Clear();

	if (onInterruptedCallback)
		onInterrupted.Bind(onInterruptedCallback);
	else
		onInterrupted.Clear();

	currentAnimation = animation;
	bPlaying = true;
}

void Animator::Stop()
{
	if (bPlaying && currentAnimation && !bFinished && onInterrupted.IsBound())
		onInterrupted.Invoke();

	bPlaying = false;
}

void Animator::Reset()
{
	currentAnimation = nullptr;
	bPlaying = false;
	ratio = 1.f;
	currentFrame = 0;
	currentTime = 0.f;
	bFlipX = false;
	bReverse = false;
	bFinished = false;

	ReleaseAnimations();
	onFinished.Clear();
	onInterrupted.Clear();
}

void Animator::SetCurrentFrame(UINT frame)
{
	if (currentAnimation == nullptr)
		return;

	UINT maxFrame = static_cast<UINT>(currentAnimation->frames.size() - 1);
	currentFrame = min(frame, maxFrame);
}

// Component Interface
void Animator::ComponentInit()
{
}

void Animator::ComponentOnEnable()
{
	Component::ComponentOnEnable();
}

void Animator::ComponentUpdate()
{
	if (!currentAnimation || !bPlaying)
		return;

	currentTime += DT;

	// 현재 프레임 시간이 지나면 다음 프레임으로
	if (currentTime >= currentAnimation->frames[currentFrame].time)
	{
		currentTime = 0.f;
		AdvanceFrame();
	}
}

void Animator::ComponentRender()
{
	if (currentAnimation == nullptr)
		return;

	Vec2 pos = GetOwner()->GetRenderPos();
	AniFrame& frame = currentAnimation->frames[currentFrame];

	float pivotX = bFlipX ? (frame.scale.x - frame.pivot.x) : frame.pivot.x;
	float pivotY = frame.pivot.y;
	float startX = pos.x - pivotX;
	float startY = pos.y - frame.scale.y + pivotY;

	RENDER->FrameImage(
		currentAnimation->image,
		startX,
		startY,
		startX + frame.scale.x * ratio,
		startY + frame.scale.y * ratio,
		frame.pos.x,
		frame.pos.y,
		frame.pos.x + frame.scale.x,
		frame.pos.y + frame.scale.y,
		bFlipX
	);
}

void Animator::ComponentOnDisable()
{
	Component::ComponentOnDisable();
}

void Animator::ComponentRelease()
{
	ReleaseAnimations();
}

// 프레임 진행
void Animator::AdvanceFrame()
{
	if (bReverse)
		AdvanceFrameReverse();
	else
		AdvanceFrameForward();
}

void Animator::AdvanceFrameForward()
{
	BroadcastFrameEvents(currentFrame);
	currentFrame++;

	UINT frameCount = static_cast<UINT>(currentAnimation->frames.size());
	if (currentFrame >= frameCount)
	{
		if (currentAnimation->repeat)
		{
			currentFrame = 0;
		}
		else
		{
			currentFrame = frameCount - 1;
			HandleAnimationEnd();
		}
	}
}

void Animator::AdvanceFrameReverse()
{
	BroadcastFrameEvents(currentFrame);

	if (currentFrame == 0)
	{
		if (currentAnimation->repeat)
		{
			currentFrame = static_cast<UINT>(currentAnimation->frames.size() - 1);
		}
		else
		{
			HandleAnimationEnd();
		}
	}
	else
	{
		currentFrame--;
	}
}

void Animator::BroadcastFrameEvents(UINT frame)
{
	if (bFinished)
		return;

	for (const wstring& eventName : currentAnimation->frames[frame].events)
	{
		OnFrameEvent.Broadcast(eventName);
	}
}

void Animator::HandleAnimationEnd()
{
	if (bFinished)
		return;

	bFinished = true;

	if (onFinished.IsBound())
		onFinished.Invoke();
}

void Animator::ReleaseAnimations()
{
	for (auto& [key, ani] : animationMap)
	{
		if (ani && !ani->IsCached())
			delete ani;
	}
	animationMap.clear();
}
