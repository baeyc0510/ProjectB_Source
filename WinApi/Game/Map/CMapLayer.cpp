#include "pch.h"
#include "CMapLayer.h"

CMapLayer::CMapLayer()
	: layerImage(nullptr)
	, parallax(1.0f)
	, offset(0, 0)
	, useTransparent(true)
{
}

CMapLayer::~CMapLayer()
{
	Release();
}

void CMapLayer::Load(const wstring& imagePath, float parallaxFactor, Vec2 offsetValue, bool transparent)
{
	Release();

	layerImage = LOADIMAGE(imagePath, imagePath);
	parallax = parallaxFactor;
	offset = offsetValue;
	useTransparent = transparent;
}

void CMapLayer::Release()
{
	// CImage는 ResourceManager가 관리하므로 여기서 삭제하지 않음
	layerImage = nullptr;
}

void CMapLayer::Render(Vec2 cameraPos)
{
	if (!layerImage)
		return;

	Vec2 virtualSize = SINGLE(CEngine)->GetVirtualSize();
	float screenW = virtualSize.x;
	float screenH = virtualSize.y;

	// Parallax 적용
	Vec2 parallaxOffset = cameraPos * parallax;

	float imgWidth = (float)layerImage->GetBmpWidth();
	float imgHeight = (float)layerImage->GetBmpHeight();

	// 화면에 그릴 위치 (왼쪽 상단 기준)
	float drawX = -parallaxOffset.x + offset.x;
	float drawY = -parallaxOffset.y + offset.y;

	// === 화면 클리핑: 보이는 영역만 렌더링 ===
	// 화면 밖이면 스킵
	if (drawX + imgWidth < 0 || drawX > screenW ||
		drawY + imgHeight < 0 || drawY > screenH)
	{
		return;
	}

	// 소스/목적지 영역 계산 (스케일 없이 1:1)
	float srcX = 0, srcY = 0;
	float srcW = imgWidth;
	float srcH = imgHeight;

	float dstX = drawX, dstY = drawY;
	float dstW = imgWidth, dstH = imgHeight;

	// 왼쪽 클리핑
	if (drawX < 0)
	{
		srcX = -drawX;
		srcW += drawX;
		dstX = 0;
		dstW += drawX;
	}

	// 위쪽 클리핑
	if (drawY < 0)
	{
		srcY = -drawY;
		srcH += drawY;
		dstY = 0;
		dstH += drawY;
	}

	// 오른쪽 클리핑
	if (dstX + dstW > screenW)
	{
		float excess = (dstX + dstW) - screenW;
		srcW -= excess;
		dstW -= excess;
	}

	// 아래쪽 클리핑
	if (dstY + dstH > screenH)
	{
		float excess = (dstY + dstH) - screenH;
		srcH -= excess;
		dstH -= excess;
	}

	// 클리핑된 영역만 렌더링 (1:1 복사)
	RENDER->FrameImage(
		layerImage,
		dstX, dstY, dstX + dstW, dstY + dstH,
		srcX, srcY, srcX + srcW, srcY + srcH,
		false,
		useTransparent ? RGB(255, 0, 255) : RGB(0, 0, 0)
	);
}

UINT CMapLayer::GetWidth() const
{
	return layerImage ? layerImage->GetBmpWidth() : 0;
}

UINT CMapLayer::GetHeight() const
{
	return layerImage ? layerImage->GetBmpHeight() : 0;
}
