#pragma once
#include "UI/CUI.h"

class CImage;

class COverlayUI : public CUI
{
public:
	COverlayUI();
	virtual ~COverlayUI();

protected:
	/*~ CUI Interfaces ~*/
	void Init() override {}
	void OnEnable() override {}
	void Update() override {}
	void Render() override {}
	void OnDisable() override {}
	void Release() override {}

	/*~ COverlayUI Interfaces ~*/
	// 원본 이미지 크기 대비 화면 비율
	Vec2 GetScaleRatio() const;

	// 원본 좌표 -> 실제 화면 좌표
	Vec2 ToScreenPos(float origX, float origY) const;
	Vec2 ToScreenPos(const Vec2& origPos) const;

	// 원본 크기 -> 실제 화면 크기
	Vec2 ToScreenSize(float origWidth, float origHeight) const;
	Vec2 ToScreenSize(const Vec2& origSize) const;

	// 전체화면으로 설정
	void SetFullscreen(CImage* img);

protected:
	Vec2 originalSize;
};
