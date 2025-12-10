#include "pch.h"
#include "RenderManager.h"

RenderManager::RenderManager()
{
	hWnd	= 0;
	hDC		= 0;
	hMemDC	= 0;
	hBMP	= 0;
	winSize	= Vec2(0.f, 0.f);
	virtualSize = Vec2(0.f, 0.f);

	hUIMemDC = 0;
	hUIBMP = 0;
	hCurrentDC = 0;
	bUIMode = false;

	hCompositeDC = 0;
	hCompositeBMP = 0;

	hFlipDC = 0;
	hFlipBMP = 0;
	flipBufWidth = 0;
	flipBufHeight = 0;

	hCurPen = 0;
	penType = PenType::Solid;
	penWidth = 1;
	penColor = RGB(0, 0, 0);

	hCurBrush = 0;
	brushType = BrushType::Solid;
	brushColor = RGB(255, 255, 255);

	hFont = 0;
	textSize = 10;
	textColor = RGB(0, 0, 0);
	textAlign = TextAlign::Center;
	textBackMode = TextBackMode::Null;
	textBackColor = RGB(255, 255, 255);
}

RenderManager::~RenderManager()
{
}

void RenderManager::Init()
{
	hWnd	= SINGLE(EngineInstance)->GetHWnd();
	winSize = SINGLE(EngineInstance)->GetWinSize();
	virtualSize = SINGLE(EngineInstance)->GetVirtualSize();
	hDC		= GetDC(SINGLE(EngineInstance)->GetHWnd());

	// 게임 백버퍼는 가상 해상도 크기로 생성 (스케일업은 EndDraw에서)
	hMemDC = CreateCompatibleDC(hDC);
	hBMP = CreateCompatibleBitmap(hDC, (int)virtualSize.x, (int)virtualSize.y);
	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemDC, hBMP));
	DeleteObject(hOldBitmap);

	// UI 백버퍼는 윈도우 해상도 크기로 생성
	hUIMemDC = CreateCompatibleDC(hDC);
	hUIBMP = CreateCompatibleBitmap(hDC, (int)winSize.x, (int)winSize.y);
	HBITMAP hOldUIBitmap = static_cast<HBITMAP>(SelectObject(hUIMemDC, hUIBMP));
	DeleteObject(hOldUIBitmap);

	// 합성용 버퍼는 윈도우 해상도 크기로 생성 (더블 버퍼링)
	hCompositeDC = CreateCompatibleDC(hDC);
	hCompositeBMP = CreateCompatibleBitmap(hDC, (int)winSize.x, (int)winSize.y);
	HBITMAP hOldCompBitmap = static_cast<HBITMAP>(SelectObject(hCompositeDC, hCompositeBMP));
	DeleteObject(hOldCompBitmap);

	// Flip용 버퍼 생성 (가상 해상도 크기면 충분)
	flipBufWidth = (int)virtualSize.x;
	flipBufHeight = (int)virtualSize.y;
	hFlipDC = CreateCompatibleDC(hDC);
	hFlipBMP = CreateCompatibleBitmap(hDC, flipBufWidth, flipBufHeight);
	HBITMAP hOldFlipBitmap = static_cast<HBITMAP>(SelectObject(hFlipDC, hFlipBMP));
	DeleteObject(hOldFlipBitmap);

	// 기본 렌더링 대상은 게임 버퍼
	hCurrentDC = hMemDC;

	SetPen();
	SetBrush();
	SetText();
	SetTextBackMode();
}

void RenderManager::BeginDraw()
{
	// 게임 버퍼로 렌더링 시작
	hCurrentDC = hMemDC;
	bUIMode = false;

	// 백버퍼(가상 해상도 크기)를 회색으로 클리어
	RECT rect = { 0, 0, (int)virtualSize.x, (int)virtualSize.y };
	HBRUSH hBrush = CreateSolidBrush(RGB(50, 50, 50));  // 어두운 회색
	FillRect(hMemDC, &rect, hBrush);
	DeleteObject(hBrush);
}

void RenderManager::EndDraw()
{
	// 1. 게임 백버퍼(가상 해상도)를 합성 버퍼로 스케일업
	SetStretchBltMode(hCompositeDC, COLORONCOLOR);
	StretchBlt(hCompositeDC, 0, 0, (int)winSize.x, (int)winSize.y,
		hMemDC, 0, 0, (int)virtualSize.x, (int)virtualSize.y, SRCCOPY);

	// 2. UI 버퍼를 합성 버퍼 위에 투명 복사 (마젠타 투명색)
	TransparentBlt(hCompositeDC, 0, 0, (int)winSize.x, (int)winSize.y,
		hUIMemDC, 0, 0, (int)winSize.x, (int)winSize.y, RGB(255, 0, 255));

	// 3. 합성 버퍼를 프론트버퍼로 한 번에 복사 (더블 버퍼링)
	BitBlt(hDC, 0, 0, (int)winSize.x, (int)winSize.y,
		hCompositeDC, 0, 0, SRCCOPY);
}

void RenderManager::BeginUI()
{
	// 이미 UI 모드면 클리어하지 않음 (중복 호출 방지)
	if (bUIMode)
	{
		hCurrentDC = hUIMemDC;
		return;
	}

	bUIMode = true;

	// UI 버퍼로 렌더링 대상 전환
	hCurrentDC = hUIMemDC;

	// UI 버퍼를 투명색(마젠타)으로 클리어
	RECT rect = { 0, 0, (int)winSize.x, (int)winSize.y };
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 255));
	FillRect(hUIMemDC, &rect, hBrush);
	DeleteObject(hBrush);
}

void RenderManager::EndUI()
{
	// 게임 버퍼로 렌더링 대상 복귀
	hCurrentDC = hMemDC;
}

void RenderManager::FinalizeUI()
{
	// 프레임 끝에서 UI 모드 리셋
	bUIMode = false;
	hCurrentDC = hMemDC;
}

void RenderManager::Release()
{
	DeleteObject(hMemDC);
	DeleteObject(hBMP);
	DeleteObject(hUIMemDC);
	DeleteObject(hUIBMP);
	DeleteObject(hCompositeDC);
	DeleteObject(hCompositeBMP);
	DeleteDC(hFlipDC);
	DeleteObject(hFlipBMP);
	ReleaseDC(hWnd, hDC);

	DeleteObject(hCurPen);
	DeleteObject(hCurBrush);

	hDC = 0;
	hMemDC = 0;
	hBMP = 0;
	hUIMemDC = 0;
	hUIBMP = 0;
	hCompositeDC = 0;
	hCompositeBMP = 0;
	hFlipDC = 0;
	hFlipBMP = 0;
}

void RenderManager::Pixel(float x, float y, COLORREF color)
{
	SetPixel(hCurrentDC, (int)x, (int)y, color);
}

void RenderManager::Line(float startX, float startY, float endX, float endY)
{
	HPEN prevPen = static_cast<HPEN>(SelectObject(hCurrentDC, hCurPen));
	HBRUSH prevBrush = static_cast<HBRUSH>(SelectObject(hCurrentDC, hCurBrush));

	MoveToEx(hCurrentDC, (int)startX, (int)startY, NULL);
	LineTo(hCurrentDC, (int)endX, (int)endY);

	SelectObject(hCurrentDC, prevPen);
	SelectObject(hCurrentDC, prevBrush);
}

void RenderManager::Rect(float startX, float startY, float endX, float endY)
{
	HPEN prevPen = static_cast<HPEN>(SelectObject(hCurrentDC, hCurPen));
	HBRUSH prevBrush = static_cast<HBRUSH>(SelectObject(hCurrentDC, hCurBrush));

	Rectangle(hCurrentDC, (int)startX, (int)startY, (int)endX, (int)endY);

	SelectObject(hCurrentDC, prevPen);
	SelectObject(hCurrentDC, prevBrush);
}

void RenderManager::Circle(float x, float y, float radius)
{
	HPEN prevPen = static_cast<HPEN>(SelectObject(hCurrentDC, hCurPen));
	HBRUSH prevBrush = static_cast<HBRUSH>(SelectObject(hCurrentDC, hCurBrush));

	::Ellipse(hCurrentDC, (int)(x - radius), (int)(y - radius), (int)(x + radius), (int)(y + radius));

	SelectObject(hCurrentDC, prevPen);
	SelectObject(hCurrentDC, prevBrush);
}

void RenderManager::Ellipse(float startX, float startY, float endX, float endY)
{
	HPEN prevPen = static_cast<HPEN>(SelectObject(hCurrentDC, hCurPen));
	HBRUSH prevBrush = static_cast<HBRUSH>(SelectObject(hCurrentDC, hCurBrush));

	::Ellipse(hCurrentDC, (int)startX, (int)startY, (int)endX, (int)endY);

	SelectObject(hCurrentDC, prevPen);
	SelectObject(hCurrentDC, prevBrush);
}

void RenderManager::Text(float x, float y, wstring str)
{
	TextOut(hCurrentDC, (int)x, (int)y, str.c_str(), (int)str.size());
}

void RenderManager::BitImage(ImageResource* pImg, float startX, float startY, float endX, float endY)
{
	BitBlt(hCurrentDC, (int)startX, (int)startY, (int)endX, (int)endY, pImg->GetImageDC(), 0, 0, SRCCOPY);
}

void RenderManager::StrectchImage(ImageResource* pImg, float startX, float startY, float endX, float endY)
{
	StretchBlt(hCurrentDC, (int)startX, (int)startY, (int)(endX - startX), (int)(endY - startY),
		pImg->GetImageDC(), 0, 0, pImg->GetBmpWidth(), pImg->GetBmpHeight(), SRCCOPY);
}

void RenderManager::TransparentImage(ImageResource* pImg, float startX, float startY, float endX, float endY, COLORREF transparent)
{
	TransparentBlt(hCurrentDC, (int)startX, (int)startY, (int)(endX - startX), (int)(endY - startY),
		pImg->GetImageDC(), 0, 0, pImg->GetBmpWidth(), pImg->GetBmpHeight(), transparent);
}

void RenderManager::FrameImage(ImageResource* pImg, float dstStartX, float dstStartY, float dstEndX, float dstEndY, float srcStartX, float srcStartY, float srcEndX, float srcEndY, bool flipX, COLORREF transparent)
{
	if (!pImg) return;

	HDC hImgDC = pImg->GetImageDC();
	int iSrcWidth = (int)(srcEndX - srcStartX);
	int iSrcHeight = (int)(srcEndY - srcStartY);
	int iDstWidth = (int)(dstEndX - dstStartX);
	int iDstHeight = (int)(dstEndY - dstStartY);

	if (flipX)
	{
		// 미리 생성된 flip 버퍼 재사용 (성능 향상)
		StretchBlt(hFlipDC, iSrcWidth - 1, 0, -iSrcWidth, iSrcHeight,
			hImgDC, (int)srcStartX, (int)srcStartY, iSrcWidth, iSrcHeight, SRCCOPY);

		TransparentBlt(hCurrentDC, (int)dstStartX, (int)dstStartY, iDstWidth, iDstHeight,
			hFlipDC, 0, 0, iSrcWidth, iSrcHeight, transparent);
	}
	else
	{
		TransparentBlt(hCurrentDC, (int)dstStartX, (int)dstStartY, iDstWidth, iDstHeight,
			hImgDC, (int)srcStartX, (int)srcStartY, iSrcWidth, iSrcHeight, transparent);
	}
}


void RenderManager::BlendImage(ImageResource* pImg, float dstStartX, float dstStartY, float dstEndX, float dstEndY, float srcStartX, float srcStartY, float srcEndX, float srcEndY, float ratio)
{
	BLENDFUNCTION bf = {};
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.AlphaFormat = 0;
	bf.SourceConstantAlpha = (BYTE)(ratio * 255);

	AlphaBlend(hCurrentDC, (int)dstStartX, (int)dstStartY, (int)(dstEndX - dstStartX), (int)(dstEndY - dstStartY),
		pImg->GetImageDC(), (int)srcStartX, (int)srcStartY, (int)(srcEndX - srcStartX), (int)(srcEndY - srcStartY), bf);
}

void RenderManager::SetPen(PenType type, COLORREF color, int width)
{
	if (penType == type && penWidth == width && penColor == color)
		return;

	penType = type;
	penWidth = width;
	penColor = color;

	DeleteObject(hCurPen);

	switch (type)
	{
	case PenType::Solid:
		hCurPen = CreatePen(PS_SOLID, width, color);
		break;
	case PenType::Dot:
		hCurPen = CreatePen(PS_DOT, width, color);
		break;
	case PenType::Dash:
		hCurPen = CreatePen(PS_DASH, width, color);
		break;
	case PenType::Null:
		hCurPen = CreatePen(PS_NULL, width, color);
		break;
	default:
		hCurPen = CreatePen(PS_SOLID, width, color);
		break;
	}
}

void RenderManager::SetBrush(BrushType type, COLORREF color)
{
	if (brushType == type && brushColor == color)
		return;

	brushType = type;
	brushColor = color;

	DeleteObject(hCurBrush);

	switch (type)
	{
	case BrushType::Solid:
		hCurBrush = CreateSolidBrush(color);
		break;
	case BrushType::Null:
		hCurBrush = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
		break;
	default:
		hCurBrush = CreateSolidBrush(color);
		break;
	}
}

void RenderManager::SetText(int size, COLORREF color, TextAlign align)
{
	if (textSize == size && textColor == color && textAlign == align)
		return;

	textSize = size;
	textColor = color;
	textAlign = align;

	DeleteObject(hFont);
	hFont = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET,
		0, 0, 0, VARIABLE_PITCH | FF_ROMAN, TEXT("굴림"));
	SelectObject(hCurrentDC, hFont);

	SetTextColor(hCurrentDC, color);

	switch (align)
	{
	case TextAlign::Top:
		SetTextAlign(hCurrentDC, TA_TOP);
		break;
	case TextAlign::Bottom:
		SetTextAlign(hCurrentDC, TA_BOTTOM);
		break;
	case TextAlign::Left:
		SetTextAlign(hCurrentDC, TA_LEFT);
		break;
	case TextAlign::Right:
		SetTextAlign(hCurrentDC, TA_RIGHT);
		break;
	case TextAlign::Center:
		SetTextAlign(hCurrentDC, TA_CENTER);
		break;
	default:
		SetTextAlign(hCurrentDC, TA_TOP);
		break;
	}

}

void RenderManager::SetTextBackMode(TextBackMode mode, COLORREF backColor)
{
	if (textBackMode == mode && textBackColor == backColor)
		return;

	switch (mode)
	{
	case TextBackMode::Null:
		SetBkMode(hCurrentDC, TRANSPARENT);
		break;
	case TextBackMode::Solid:
		SetBkMode(hCurrentDC, OPAQUE);
		break;

	default:
		SetBkMode(hCurrentDC, TRANSPARENT);
		break;
	}
}
