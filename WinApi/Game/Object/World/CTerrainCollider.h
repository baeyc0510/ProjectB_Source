#pragma once

// 지형 콜라이더 오브젝트 - CMap에서 생성하는 지형 충돌 영역
class CTerrainCollider : public CGameObject
{
public:
	CTerrainCollider();
	virtual ~CTerrainCollider();

	void SetBoxCollider(const Vec2& center, const Vec2& size);
	void SetLineCollider(const Vec2& start, const Vec2& end);

	CCollider* GetCollider() { return collider; }

private:
	void Init() override;
	void OnEnable() override;
	void Update() override;
	void OnDisable() override;
	void Release() override;
	void Render() override;

private:
	CCollider* collider = nullptr;
	bool isLineCollider = false;
};
