#pragma once
#include "Game/Enum.h"

// 월드 콜라이더 베이스 클래스 - 지형 충돌 오브젝트의 공통 기능
class CWorldCollider : public CGameObject
{
public:
	CWorldCollider();
	virtual ~CWorldCollider();

	CCollider* GetCollider() { return collider; }

protected:
	void SetBoxCollider(const Vec2& center, const Vec2& size, ELayer layer);
	void SetLineCollider(const Vec2& start, const Vec2& end, ELayer layer);

	void Init() override {}
	void OnEnable() override {}
	void Update() override {}
	void OnDisable() override {}
	void Release() override {}
	void Render() override {}

protected:
	CCollider* collider = nullptr;
	bool isLineCollider = false;
};
