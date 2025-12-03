#pragma once
#include "CCollider.h"

class CLineCollider : public CCollider
{
public:
	CLineCollider();
	virtual ~CLineCollider();

public:
	// 선분 설정 (로컬 좌표)
	void SetLine(const Vec2& start, const Vec2& end);
	void SetStart(const Vec2& start) { localStart = start; }
	void SetEnd(const Vec2& end) { localEnd = end; }

	// 월드 좌표로 변환된 선분 가져오기
	Vec2 GetWorldStart() const;
	Vec2 GetWorldEnd() const;

	// X 위치에서의 Y 높이 계산 (슬로프용)
	float GetYAt(float worldX) const;

	// Y 위치에서의 X 좌표 계산 (GetYAt의 역함수)
	float GetXAt(float worldY) const;

	// X가 선분 범위 내인지 확인
	bool IsInXRange(float worldX) const;

	// 점이 선분 위에 있는지 체크 (toleranceY: 허용 오차)
	bool IsPointOnLine(const Vec2& point, float toleranceY = 5.0f) const;

	// 슬로프 각도 반환 (라디안, 수평=0, 오르막=양수)
	float GetSlopeAngle() const;

	// 슬로프 각도 반환 (도 단위)
	float GetSlopeAngleDegrees() const;

	Vec2 GetScale() const override;
	void SetScale(const Vec2& scale) override;
	EColliderType GetType() const override { return EColliderType::Line; }

private:
	void Render() override;

	Vec2 localStart;
	Vec2 localEnd;
};
