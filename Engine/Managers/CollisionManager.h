#pragma once

struct HitResult;
class Collider;
class BoxCollider;
class LineCollider;

struct DebugBox
{
	Vec2 center;
	Vec2 halfSize;
	COLORREF color;
	float duration;
};

class CollisionManager : public SingleTon<CollisionManager>
{
	friend SingleTon<CollisionManager>;
private:
	CollisionManager();
	virtual ~CollisionManager();

public:
	static const UINT MAXLAYER = 64;

public:
	void	Init();
	void	Update();
	void	Release();

	void	AddCollider(Collider* collider);
	void	RemoveCollider(Collider* collider);

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

	// 충돌 판정 함수 (타입별)
	static bool CheckCollision(Collider* left, Collider* right);
	static bool BoxVsBox(BoxCollider* a, BoxCollider* b);
	static bool BoxVsLine(BoxCollider* box, LineCollider* line);
	static bool LineVsLine(LineCollider* a, LineCollider* b);

	array<list<Collider*>, MAXLAYER>		colliderList;
	array<array<bool, MAXLAYER>, MAXLAYER>	layerMask;
	unordered_map<UINT64, bool>				prevCollision;

	vector<DebugBox>						debugBoxes;
};

#define COLLISION CollisionManager::GetInstance()
