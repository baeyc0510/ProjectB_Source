#pragma once
class EngineInstance : public SingleTon<EngineInstance>
{
	friend SingleTon<EngineInstance>;
private:
	EngineInstance();
	virtual ~EngineInstance();

public:
	void		Init(HINSTANCE hInst, HWND hWnd, Vec2 winSize, Vec2 virtualSize = Vec2(0, 0));
	void		Release();

	HINSTANCE	GetHInst()			{ return hInst; }
	HWND		GetHWnd()			{ return hWnd; }
	Vec2		GetWinSize()		{ return winSize; }
	Vec2		GetVirtualSize()	{ return virtualSize; }
	float		GetRenderScale()	{ return renderScale; }

private:
	HINSTANCE	hInst;
	HWND		hWnd;
	Vec2		winSize;		// 실제 윈도우 크기
	Vec2		virtualSize;	// 가상 해상도
	float		renderScale;	// 렌더 스케일
};

