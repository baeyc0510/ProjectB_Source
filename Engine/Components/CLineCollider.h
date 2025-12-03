#pragma once
#include "CCollider.h"

// 선분(라인) 콜라이더 - 슬로프/경사면 지형 충돌용
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

	// X가 선분 범위 내인지 확인
	bool IsInXRange(float worldX) const;

	// 박스와 선분의 충돌 체크
	bool IsCollisionWithBox(const Vec2& boxPos, const Vec2& boxScale) const;

	// 점이 선분 위에 있는지 체크 (toleranceY: 허용 오차)
	bool IsPointOnLine(const Vec2& point, float toleranceY = 5.0f) const;

private:
	virtual bool IsCollision(CCollider* other) override;
	void Render() override;

private:
	Vec2 localStart;  // 로컬 시작점
	Vec2 localEnd;    // 로컬 끝점
};
