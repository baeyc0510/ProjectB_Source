#pragma once
#include "Game/Enum.h"

class CImage;

// 메타데이터 색상 정의
namespace MetaColor
{
	constexpr COLORREF Empty     = RGB(0, 0, 0);        // 검정 - 빈 공간
	constexpr COLORREF Solid     = RGB(255, 0, 0);      // 빨강 - 솔리드
	constexpr COLORREF OneWay    = RGB(0, 255, 0);      // 초록 - 원웨이 플랫폼
	constexpr COLORREF Ladder    = RGB(0, 0, 255);      // 파랑 - 사다리
	constexpr COLORREF WallClimb = RGB(255, 128, 0);    // 주황 - 벽타기
	constexpr COLORREF Ledge     = RGB(0, 255, 255);    // 청록 - 매달리기
	constexpr COLORREF Swamp     = RGB(128, 0, 128);    // 보라 - 늪
	constexpr COLORREF Damage    = RGB(255, 255, 0);    // 노랑 - 데미지
}

class CMetaMap
{
public:
	CMetaMap();
	~CMetaMap();

	void Load(const wstring& path);
	void Release();

	// 크기 정보
	UINT GetWidth() const { return width; }
	UINT GetHeight() const { return height; }
	bool IsLoaded() const { return pixelData != nullptr; }

	// 픽셀 색상 조회 (좌표가 범위 밖이면 Solid 반환)
	COLORREF GetPixelColor(int x, int y) const;
	COLORREF GetPixelColor(Vec2 pos) const;

	// 지형 타입 조회
	ETerrain GetTerrain(int x, int y) const;
	ETerrain GetTerrain(Vec2 pos) const;

	// 지형 타입 체크 헬퍼
	bool IsEmpty(int x, int y) const     { return GetTerrain(x, y) == ETerrain::Empty; }
	bool IsSolid(int x, int y) const     { return GetTerrain(x, y) == ETerrain::Solid; }
	bool IsOneWay(int x, int y) const    { return GetTerrain(x, y) == ETerrain::OneWay; }
	bool IsLadder(int x, int y) const    { return GetTerrain(x, y) == ETerrain::Ladder; }
	bool IsWallClimb(int x, int y) const { return GetTerrain(x, y) == ETerrain::WallClimb; }
	bool IsLedge(int x, int y) const     { return GetTerrain(x, y) == ETerrain::Ledge; }
	bool IsSwamp(int x, int y) const     { return GetTerrain(x, y) == ETerrain::Swamp; }
	bool IsDamage(int x, int y) const    { return GetTerrain(x, y) == ETerrain::Damage; }

	// 충돌 체크 (Solid 또는 OneWay)
	bool IsBlocking(int x, int y) const;
	bool IsGroundAt(int x, int y, bool checkOneWay = true) const;

private:
	ETerrain ColorToTerrain(COLORREF color) const;
	void CachePixelData(CImage* image);

private:
	BYTE* pixelData;	// 캐싱된 픽셀 데이터 (BGR 포맷)
	UINT width;
	UINT height;
	UINT stride;		// 행당 바이트 수 (4바이트 정렬)
};
