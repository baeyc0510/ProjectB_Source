#pragma once
#include "Util/Delegate.h"

class CAnimation;
class CImage;

class CAnimator : public Component<CGameObject>
{
public:
	CAnimator();
	virtual ~CAnimator();

	void			Reset();
	void			AddAnimation(const wstring& aniName, CAnimation* animation);
	void			RemoveAnimation(const wstring& aniName);
	CAnimation*		FindAnimation(const wstring& aniName);
	void			CreateAnimation(const wstring& aniName, CImage* image,
						float stepTime, UINT count, bool repeat,
						Vec2 pos, Vec2 scale, Vec2 step);

	void			Play(const wstring& aniName, bool reset = false,
						Delegate<>::EventFunc onFinished = nullptr,
						Delegate<>::EventFunc onInterrupted = nullptr);
	void			Stop();

	bool			IsFinished() const		{ return isFinished; }
	float			GetRatio()				{ return ratio; }
	void			SetRatio(float ratio)	{ this->ratio = ratio; }
	void			SetDirection(int dir)	{ flipX =  dir <= 0; }
	void			SetReverse(bool reverse){ this->reverse = reverse; }
	bool			IsReverse() const		{ return reverse; }

	UINT			GetCurrentFrame()			{ return curFrame; }
	void			SetCurrentFrame(UINT inFrame);


	// 프레임 이벤트 (AbilitySystem 연동용)
	MulticastDelegate<const wstring&> OnFrameEvent;

private:
	void			ComponentInit()			override;
	void			ComponentOnEnable()		override;
	void			ComponentUpdate()		override;
	void			ComponentRender()		override;
	void			ComponentOnDisable()	override;
	void ReleaseAnimations();
	void			ComponentRelease()		override;

	map<wstring, CAnimation*>		animationMap;
	CAnimation*						curAnimation;
	bool							playing;
	float							ratio;

	UINT							curFrame;
	float							curTime;
	bool							flipX;
	bool							reverse;
	bool							isFinished;

	// 현재 애니메이션에 대한 콜백
	Delegate<>						onFinished;
	Delegate<>						onInterrupted;
};

