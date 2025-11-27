#pragma once

struct HitResult;
class CCollider;

struct DebugBox
{
	Vec2 center;
	Vec2 halfSize;
	COLORREF color;
	float duration;
};

class CCollisionManager : public SingleTon<CCollisionManager>
{
	friend SingleTon<CCollisionManager>;
private:
	CCollisionManager();
	virtual ~CCollisionManager();

public:
	static const UINT MAXLAYER = 64;

public:
	void	Init();
	void	Update();
	void	Release();

	void	AddCollider(CCollider* collider);
	void	RemoveCollider(CCollider* collider);

	void	CheckLayer(UINT left, UINT right);
	void	UnCheckLayer(UINT left, UINT right);
	void	ResetCheck();

	// Box Trace
	vector<HitResult> BoxTrace(const Vec2& center, const Vec2& halfSize, UINT targetLayer, bool bDrawDebug = false);

	// Debug Draw
	void	DrawDebugBox(const Vec2& center, const Vec2& halfSize,
						 COLORREF color = RGB(0, 255, 0), float duration = 0.1f);
	void	RenderDebug();

private:
	void	CollisionUpdate(UINT left, UINT right);
	UINT64	CollisionID(UINT leftID, UINT rightID);

	array<list<CCollider*>, MAXLAYER>		colliderList;
	array<array<bool, MAXLAYER>, MAXLAYER>	layerMask;
	unordered_map<UINT64, bool>				prevCollision;

	vector<DebugBox>						debugBoxes;
};

#define COLLISION CCollisionManager::GetInstance()
