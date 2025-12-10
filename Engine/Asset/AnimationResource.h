#pragma once
#include "ResourceBase.h"

class Animator;
class ImageResource;


struct AniFrame
{
	Vec2	pos;
	Vec2	scale;
	Vec2	pivot;
	float	time;
	vector<wstring> events;
};

class AnimationResource : public ResourceBase
{
	friend Animator;
public:
	AnimationResource();
	virtual ~AnimationResource();

public:
	const static UINT	FRAME_SIZE = 128;

	void				Load(const wstring& key, const wstring& path);
	void				SetRepeat(bool repeat) { this->repeat = repeat; }
	void				Create(ImageResource* image, float stepTime, UINT count, bool repeat,
							Vec2 pos, Vec2 scale, Vec2 step);	// 일정 간격으로 프레임 생성
	bool				IsCached() const { return this->cached; }
	void				SetCached(bool value) { this->cached = value; }
	
private:
	ImageResource*				image;		// 애니메이션 이미지
	vector<AniFrame>	frames;		// 애니메이션 프레임들
	bool				repeat;		// 애니메이션 반복여부
	bool				cached;		// 매니저 캐시 여부
};

