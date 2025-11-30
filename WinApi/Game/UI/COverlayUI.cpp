#include "pch.h"
#include "COverlayUI.h"
#include "Asset/CImage.h"

COverlayUI::COverlayUI()
	: originalSize(0.f, 0.f)
{
}

COverlayUI::~COverlayUI()
{
}

Vec2 COverlayUI::GetScaleRatio() const
{
	if (originalSize.x <= 0.f || originalSize.y <= 0.f)
		return Vec2(1.f, 1.f);

	return Vec2(scale.x / originalSize.x, scale.y / originalSize.y);
}

Vec2 COverlayUI::ToScreenPos(float origX, float origY) const
{
	Vec2 ratio = GetScaleRatio();
	return Vec2(renderPos.x + origX * ratio.x, renderPos.y + origY * ratio.y);
}

Vec2 COverlayUI::ToScreenPos(const Vec2& origPos) const
{
	return ToScreenPos(origPos.x, origPos.y);
}

Vec2 COverlayUI::ToScreenSize(float origWidth, float origHeight) const
{
	Vec2 ratio = GetScaleRatio();
	return Vec2(origWidth * ratio.x, origHeight * ratio.y);
}

Vec2 COverlayUI::ToScreenSize(const Vec2& origSize) const
{
	return ToScreenSize(origSize.x, origSize.y);
}

void COverlayUI::SetFullscreen(CImage* img)
{
	if (img)
	{
		// 원본 사이즈 저장
		originalSize = Vec2((float)img->GetBmpWidth(), (float)img->GetBmpHeight());
	}

	pos = Vec2(0.f, 0.f);
	scale = SINGLE(CEngine)->GetWinSize();  // 실제 클라이언트 크기 사용
}
