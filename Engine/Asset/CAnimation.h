#pragma once
#include "CResource.h"

class CAnimator;
class CImage;


struct AniFrame
{
	Vec2	pos;
	Vec2	scale;
	float	time;
	vector<wstring> events;
};

class CAnimation : public CResource
{
	friend CAnimator;
public:
	CAnimation();
	virtual ~CAnimation();

public:
	const static UINT	FRAME_SIZE = 128;

	void				Load(const wstring& key, const wstring& path);
	void				SetRepeat(bool repeat) { this->repeat = repeat; }
	void				Create(CImage* image, float stepTime, UINT count, bool repeat,
							Vec2 pos, Vec2 scale, Vec2 step);	// 일정 간격으로 프레임 생성

private:
	CImage*				image;		// 애니메이션 이미지
	vector<AniFrame>	frames;		// 애니메이션 프레임들
	bool				repeat;		// 애니메이션 반복여부
};

