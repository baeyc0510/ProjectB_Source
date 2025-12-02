#include "pch.h"
#include "CMetaMap.h"

CMetaMap::CMetaMap()
	: pixelData(nullptr)
	, width(0)
	, height(0)
	, stride(0)
{
}

CMetaMap::~CMetaMap()
{
	Release();
}

void CMetaMap::Load(const wstring& path)
{
	Release();

	CImage* metaImage = LOADIMAGE(path, path);
	if (metaImage)
	{
		width = metaImage->GetBmpWidth();
		height = metaImage->GetBmpHeight();
		CachePixelData(metaImage);
	}
}

void CMetaMap::CachePixelData(CImage* image)
{
	if (!image)
		return;

	HDC hdc = image->GetImageDC();
	HBITMAP hBmp = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);

	// 비트맵 정보 설정
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -(LONG)height;  // 위에서 아래로
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	// 행당 바이트 수 (4바이트 정렬)
	stride = ((width * 3 + 3) / 4) * 4;

	// 픽셀 데이터 버퍼 할당
	pixelData = new BYTE[stride * height];

	// GetDIBits로 픽셀 데이터 복사
	GetDIBits(hdc, hBmp, 0, height, pixelData, &bmi, DIB_RGB_COLORS);
}

void CMetaMap::Release()
{
	if (pixelData)
	{
		delete[] pixelData;
		pixelData = nullptr;
	}
	width = 0;
	height = 0;
	stride = 0;
}

COLORREF CMetaMap::GetPixelColor(int x, int y) const
{
	if (!pixelData)
		return MetaColor::Empty;
	
	// 범위 체크
	if (y < 0 || y >= (int)height)
		return MetaColor::Empty;
	
	if (x < 0 || x >= (int)width)
		return MetaColor::Solid;
	
	// 픽셀 위치 계산 (탑-다운)
	BYTE* pixel = pixelData + y * stride + x * 3;
	BYTE b = pixel[0];
	BYTE g = pixel[1];
	BYTE r = pixel[2];

	return RGB(r, g, b);
}

COLORREF CMetaMap::GetPixelColor(Vec2 pos) const
{
	return GetPixelColor((int)pos.x, (int)pos.y);
}

ETerrain CMetaMap::GetTerrain(int x, int y) const
{
	return ColorToTerrain(GetPixelColor(x, y));
}

ETerrain CMetaMap::GetTerrain(Vec2 pos) const
{
	return GetTerrain((int)pos.x, (int)pos.y);
}

bool CMetaMap::IsBlocking(int x, int y) const
{
	ETerrain terrain = GetTerrain(x, y);
	return terrain == ETerrain::Solid || terrain == ETerrain::OneWay;
}

bool CMetaMap::IsGroundAt(int x, int y, bool checkOneWay) const
{
	ETerrain terrain = GetTerrain(x, y);
	if (terrain == ETerrain::Solid)
		return true;
	if (checkOneWay && terrain == ETerrain::OneWay)
		return true;
	return false;
}

ETerrain CMetaMap::ColorToTerrain(COLORREF color) const
{
	if (color == MetaColor::Empty)     return ETerrain::Empty;
	if (color == MetaColor::Solid)     return ETerrain::Solid;
	if (color == MetaColor::OneWay)    return ETerrain::OneWay;
	if (color == MetaColor::Ladder)    return ETerrain::Ladder;
	if (color == MetaColor::WallClimb) return ETerrain::WallClimb;
	if (color == MetaColor::Ledge)     return ETerrain::Ledge;
	if (color == MetaColor::Swamp)     return ETerrain::Swamp;
	if (color == MetaColor::Damage)    return ETerrain::Damage;

	// 알 수 없는 색상
	return ETerrain::Empty;
}