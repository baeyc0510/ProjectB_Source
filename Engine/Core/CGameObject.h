#pragma once
#include "Component.h"
class CWorldManager;
class CScene;

class CGameObject : public Composite<CGameObject>, public IRender
{
	friend CWorldManager;
	friend CScene;
	friend CCollider;
public:
	CGameObject();
	virtual ~CGameObject();

public:
	const wstring&	GetName()						{ return name; }
	const Vec2&		GetPos()						{ return pos; }
	const Vec2&		GetWorldPos()					{ return worldPos; }
	const Vec2&		GetRenderPos()					{ return renderPos; }
	const Vec2&		GetScale()						{ return scale; }

	void			SetName(const wstring& name)	{ this->name = name; }
	void			SetPos(const Vec2& pos)			{ this->pos = pos; }
	void			SetScale(const Vec2& scale)		{ this->scale = scale; }
	
	int				GetForward()					{ return forward; }
	void			SetForward(int inForward)		{ this->forward = inForward > 0 ? 1 : -1; }
	void			SetForward(float inForward)		{ this->forward = inForward > 0 ? 1 : -1; }
	
protected:
	wstring			name;
	Vec2			pos;
	Vec2			worldPos;
	Vec2			renderPos;
	Vec2			scale;
	int				forward;
	// float		zOrder;

private:
	virtual void	Init()		= 0;
	virtual void	OnEnable()	= 0;
	virtual void	Update()	= 0;
	//virtual void	Render()	= 0;
	virtual void	OnDisable()	= 0;
	virtual void	Release()	= 0;

	void			ComponentInit()			override;
	void			ComponentOnEnable()		override;
	void			ComponentUpdate()		override;
	void			ComponentRender()		override;
	void			ComponentOnDisable()	override;
	void			ComponentRelease()		override;

	virtual void	OnCollisionEnter(CCollider* other) {};
	virtual void	OnCollisionStay(CCollider* other) {};
	virtual void	OnCollisionExit(CCollider* other) {};
};

