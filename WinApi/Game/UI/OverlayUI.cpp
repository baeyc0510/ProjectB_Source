#include "pch.h"
#include "OverlayUI.h"
#include "Asset/ImageResource.h"

OverlayUI::OverlayUI()
	: originalSize(0.f, 0.f)
{
}

OverlayUI::~OverlayUI()
{
}

Vec2 OverlayUI::GetScaleRatio() const
{
	if (originalSize.x <= 0.f || originalSize.y <= 0.f)
		return Vec2(1.f, 1.f);

	return Vec2(scale.x / originalSize.x, scale.y / originalSize.y);
}

Vec2 OverlayUI::ToScreenPos(float origX, float origY) const
{
	Vec2 ratio = GetScaleRatio();
	return Vec2(renderPos.x + origX * ratio.x, renderPos.y + origY * ratio.y);
}

Vec2 OverlayUI::ToScreenPos(const Vec2& origPos) const
{
	return ToScreenPos(origPos.x, origPos.y);
}

Vec2 OverlayUI::ToScreenSize(float origWidth, float origHeight) const
{
	Vec2 ratio = GetScaleRatio();
	return Vec2(origWidth * ratio.x, origHeight * ratio.y);
}

Vec2 OverlayUI::ToScreenSize(const Vec2& origSize) const
{
	return ToScreenSize(origSize.x, origSize.y);
}

void OverlayUI::SetFullscreen(ImageResource* img)
{
	if (img)
	{
		// 원본 사이즈 저장
		originalSize = Vec2((float)img->GetBmpWidth(), (float)img->GetBmpHeight());
	}

	pos = Vec2(0.f, 0.f);
	scale = SINGLE(EngineInstance)->GetWinSize();  // UI는 윈도우 해상도 기준
}
