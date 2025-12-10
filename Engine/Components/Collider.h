#pragma once

class CollisionManager;

enum class EColliderType
{
	Box,
	Line
};

class Collider : public Component<GameObject>, public IRender
{
	friend CollisionManager;
public:
	Collider();
	virtual ~Collider();

public:
	UINT				GetID() const					{ return id; }
	UINT				GetLayer() const				{ return layer; }
	const Vec2&			GetPos() const					{ return pos; }
	const Vec2&			GetOffset() const				{ return offset; }
	virtual Vec2		GetScale() const = 0;
	virtual EColliderType GetType() const = 0;

	void				SetLayer(UINT layer)			{ this->layer = layer; }
	void				SetPos(const Vec2& pos)			{ this->pos = pos; }
	void				SetOffset(const Vec2& offset)	{ this->offset = offset; }
	virtual void		SetScale(const Vec2& scale)		= 0;
	
	virtual void		RenderDebug();
protected:
	static const COLORREF layerColors[16];
	bool				IsColliding() const				{ return count > 0; }

	void				ComponentInit()					override;
	void				ComponentOnEnable()				override;
	void				ComponentUpdate()				override;
	void				ComponentRender()				override;
	void				ComponentOnDisable()			override;
	void				ComponentRelease()				override;

	static UINT			colliderCount;
	UINT				id;
	UINT				count;
	UINT				layer;
	Vec2				pos;
	Vec2				offset;

private:
	void				OnCollisionEnter(Collider* other);
	void				OnCollisionStay(Collider* other);
	void				OnCollisionExit(Collider* other);
};
