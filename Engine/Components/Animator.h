#pragma once
#include "Util/Delegate.h"

class AnimationResource;
class ImageResource;

class Animator : public Component<GameObject>
{
public:
	Animator();
	virtual ~Animator();

	/*~ Animation 관리 ~*/
	void			AddAnimation(const wstring& aniName, AnimationResource* animation);
	void			RemoveAnimation(const wstring& aniName);
	AnimationResource*		FindAnimation(const wstring& aniName);
	void			CreateAnimation(const wstring& aniName, ImageResource* image,
						float stepTime, UINT count, bool repeat,
						Vec2 pos, Vec2 scale, Vec2 step);

	/*~ 재생 제어 ~*/
	void			Play(const wstring& aniName, bool reset = false,
						Delegate<>::EventFunc onFinished = nullptr,
						Delegate<>::EventFunc onInterrupted = nullptr);
	void			Stop();
	void			Reset();

	/*~ Getter/Setter ~*/
	bool			IsFinished() const		{ return bFinished; }
	bool			IsReverse() const		{ return bReverse; }
	UINT			GetCurrentFrame() const	{ return currentFrame; }
	float			GetRatio() const		{ return ratio; }

	void			SetRatio(float value)	{ ratio = value; }
	void			SetDirection(int dir)	{ bFlipX = dir <= 0; }
	void			SetReverse(bool value)	{ bReverse = value; }
	void			SetCurrentFrame(UINT frame);

	/*~ 이벤트 ~*/
	MulticastDelegate<const wstring&> OnFrameEvent;

protected:
	/*~ Component Interface ~*/
	void			ComponentInit() override;
	void			ComponentOnEnable() override;
	void			ComponentUpdate() override;
	void			ComponentRender() override;
	void			ComponentOnDisable() override;
	void			ComponentRelease() override;

private:
	void			AdvanceFrame();
	void			AdvanceFrameForward();
	void			AdvanceFrameReverse();
	void			BroadcastFrameEvents(UINT frame);
	void			HandleAnimationEnd();
	void			ReleaseAnimations();

private:
	map<wstring, AnimationResource*> animationMap;
	AnimationResource* currentAnimation = nullptr;

	UINT	currentFrame = 0;
	float	currentTime = 0.f;
	float	ratio = 1.f;

	bool	bPlaying = false;
	bool	bFinished = false;
	bool	bFlipX = false;
	bool	bReverse = false;

	Delegate<> onFinished;
	Delegate<> onInterrupted;
};
