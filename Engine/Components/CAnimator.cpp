#include "pch.h"
#include "CAnimator.h"

CAnimator::CAnimator()
{
	curAnimation	= nullptr;
	playing			= false;
	ratio			= 1;
	curFrame		= 0;
	curTime			= 0;
	flipX			= false;
	reverse			= false;
	isFinished		= false;
}

CAnimator::~CAnimator()
{
}

void CAnimator::Reset()
{
	curAnimation	= nullptr;
	playing			= false;
	ratio			= 1;
	curFrame		= 0;
	curTime			= 0;
	flipX			= false;
	reverse			= false;
	isFinished		= false;

	ReleaseAnimations();
	onFinished.Clear();
	onInterrupted.Clear();
}

void CAnimator::AddAnimation(const wstring& aniName, CAnimation* animation)
{
	CAnimation* ani = FindAnimation(aniName);
	assert(nullptr == ani && "Animation already exist");

	animation->SetKey(aniName);
	animation->SetPath(TEXT(""));
	animationMap.insert(make_pair(aniName, animation));
}

void CAnimator::RemoveAnimation(const wstring& aniName)
{
	CAnimation* ani = FindAnimation(aniName);

	if (nullptr != ani)
	{
		animationMap.erase(aniName);
		if (!ani->IsCached())
		{
			delete ani;
		}
	}
}

CAnimation* CAnimator::FindAnimation(const wstring& aniName)
{
	auto iter = animationMap.find(aniName);
	if (iter == animationMap.end())
		return nullptr;
	else
		return iter->second;
}

void CAnimator::CreateAnimation(const wstring& aniName, CImage* image, float stepTime, UINT count, bool repeat, Vec2 pos, Vec2 scale, Vec2 step)
{
	CAnimation* ani = new CAnimation();
	ani->Create(image, stepTime, count, repeat, pos, scale, step);
	AddAnimation(aniName, ani);
}

void CAnimator::Play(const wstring& aniName, bool reset,
	Delegate<>::EventFunc onFinishedCallback,
	Delegate<>::EventFunc onInterruptedCallback)
{
	// 현재 애니메이션이 플레이하고자 하는 애니메이션이며
	// reset 아닐 경우 현재 애니메이션을 변경하지 않음
	// reset : 같은 애니메이션을 처음부터 재생
	// (ex. 공격 모션처럼 누를때마다 처음부터 재생해야하는 애니메이션)
	if (playing && aniName == curAnimation->GetKey() && !reset)
		return;

	CAnimation* animation = FindAnimation(aniName);
	assert(nullptr != animation && "Animation no exist");

	// 애니메이션이 변경되는 경우 (중단)
	if (curAnimation != nullptr && curAnimation != animation)
	{
		// 이전 애니메이션이 완료되지 않았다면 중단 이벤트 호출
		if (!isFinished && onInterrupted.IsBound())
		{
			onInterrupted.Invoke();
		}
	}

	// reset 일 경우 처음부터 재생 (역재생이면 마지막 프레임에서 시작)
	if (reset || curAnimation != animation)
	{
		curFrame = reverse ? static_cast<UINT>(animation->frames.size() - 1) : 0;
		curTime = 0;
		isFinished = false;
	}

	// 새로운 콜백 등록
	if (onFinishedCallback)
		onFinished.Bind(onFinishedCallback);
	else
		onFinished.Clear();

	if (onInterruptedCallback)
		onInterrupted.Bind(onInterruptedCallback);
	else
		onInterrupted.Clear();

	curAnimation = animation;
	playing = true;
}

void CAnimator::Stop()
{
	// 애니메이션이 완료되지 않았다면 중단 이벤트 호출
	if (playing && curAnimation != nullptr && !isFinished && onInterrupted.IsBound())
	{
		onInterrupted.Invoke();
	}

	playing = false;
}

void CAnimator::SetCurrentFrame(UINT inFrame)
{
	if (curAnimation == nullptr)
	{
		return;
	}
	
	UINT maxFrame = static_cast<UINT>(curAnimation->frames.size() - 1);
	curFrame = max(inFrame, maxFrame);
}

void CAnimator::ComponentInit()
{
}

void CAnimator::ComponentOnEnable()
{
	Component::ComponentOnEnable();
}

void CAnimator::ComponentUpdate()
{
	if (curAnimation == nullptr)
	{
		return;
	}
	if (!playing)
	{
		return;
	}

	// 현재 플레이중인 프레임의 누적시간
	curTime += DT;

	// 누적시간이 현재 플레이중인 프레임의 지속시간보다 커졌을 경우
	// -> 다음 프레임을 보여줘야 하는 경우
	if (curAnimation->frames[curFrame].time < curTime)
	{
		curTime = 0;	// 현재 플레이중인 프레임의 누적시간 초기화

		// 역재생
		if (reverse)
		{
			if (curFrame == 0)
			{
				// 첫 프레임에 도달
				if (curAnimation->repeat)
				{
					curFrame = static_cast<UINT>(curAnimation->frames.size() - 1);
				}
				else
				{
					// 반복 아니면 0에서 멈춤
					if (isFinished)
					{
						return;
					}

					isFinished = true;

					if (onFinished.IsBound())
					{
						onFinished.Invoke();
					}
				}
			}
			else
			{
				curFrame--;
			}
		}
		// 정방향 재생
		else
		{
			UINT prevFrame = curFrame;
			curFrame++;

			if (!isFinished)
			{
				// 프레임 이벤트 실행
				for (const wstring& eventName : curAnimation->frames[prevFrame].events)
				{
					OnFrameEvent.Broadcast(eventName);
				}
			}

			// 만약 플레이중인 프레임이 마지막 프레임이었을 경우
			if (curFrame == curAnimation->frames.size())
			{
				// 반복 애니메이션이라면 처음부터, 아니라면 마지막을 다시 재생
				if (curAnimation->repeat)
				{
					curFrame = 0;
				}
				else
				{
					curFrame--;

					if (isFinished)
					{
						return;
					}

					isFinished = true;

					// 애니메이션 완료 이벤트 호출
					if (onFinished.IsBound())
					{
						onFinished.Invoke();
					}
				}
			}
		}
	}
}

void CAnimator::ComponentRender()
{
	if (curAnimation == nullptr)
	{
		return;
	}
	
	Vec2 pos = GetOwner()->GetRenderPos();				// 애니메이션이 그려질 위치 확인
	AniFrame frame = curAnimation->frames[curFrame];	// 애니메이션이 그려질 프레임 확인
	
	float pivotX = !flipX ?  frame.pivot.x : frame.scale.x - frame.pivot.x;
	float pivotY = frame.pivot.y;
	float startX = pos.x - pivotX;
	float startY = pos.y - frame.scale.y + pivotY;
	
	RENDER->FrameImage(
		curAnimation->image,
		startX,
		startY,
		startX + frame.scale.x * ratio,
		startY + frame.scale.y * ratio,
		frame.pos.x,
		frame.pos.y,
		frame.pos.x + frame.scale.x,
		frame.pos.y + frame.scale.y,
		flipX
	);
}

void CAnimator::ComponentOnDisable()
{
	Component::ComponentOnDisable();
}

void CAnimator::ReleaseAnimations()
{
	for (pair<wstring, CAnimation*> kvp : animationMap)
	{
		auto ani = kvp.second;
		if (ani != nullptr && !ani->IsCached())
		{
			delete ani;
		}
	}
	animationMap.clear();
}

void CAnimator::ComponentRelease()
{
	ReleaseAnimations();
}
