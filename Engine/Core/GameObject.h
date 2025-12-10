#pragma once
#include "Component.h"
class WorldManager;
class Scene;

class Collider;

class GameObject : public Composite<GameObject>, public IRender
{
	friend WorldManager;
	friend Scene;
	friend Collider;
public:
	GameObject();
	virtual ~GameObject();

public:
	const wstring&	GetName()						{ return name; }
	const Vec2&		GetPos()						{ return pos; }
	const Vec2&		GetWorldPos()					{ return worldPos; }
	const Vec2&		GetRenderPos()					{ return renderPos; }
	const Vec2&		GetScale()						{ return scale; }

	void			SetName(const wstring& name)	{ this->name = name; }
	void			SetPos(const Vec2& pos)			{ this->pos = pos; }
	void			SetScale(const Vec2& scale)		{ this->scale = scale; }

	int				GetForward() const { return forward; }
	void			SetForward(int inForward)		{ this->forward = inForward > 0 ? 1 : -1; }
	void			SetForward(float inForward)		{ this->forward = inForward > 0 ? 1 : -1; }

	// Persistent: 씬 전환 시에도 유지되는 오브젝트
	bool			IsPersistent() const			{ return isPersistent; }
	void			SetPersistent(bool value)		{ isPersistent = value; }

	// Lifetime: 일정 시간 후 자동 삭제
	void			SetLifetime(float seconds);
	float			GetLifetime() const				{ return lifetime; }
	float			GetRemainingLifetime() const	{ return remainingLifetime; }
	bool			HasLifetime() const				{ return bHasLifetime; }

	// 파괴 요청 (OnDestroy 호출 후 삭제 예약)
	void			Destroy();

protected:
	// 오브젝트 파괴 시 호출 (파생 클래스에서 오버라이드)
	virtual void	OnDestroy() {}

protected:
	wstring			name;
	Vec2			pos;
	Vec2			worldPos;
	Vec2			renderPos;
	Vec2			scale;
	int				forward;
	bool			isPersistent = false;
	// float		zOrder;

private:
	virtual void	Init()		= 0;
	virtual void	OnEnable()	= 0;
	virtual void	OnDisable()	= 0;
	virtual void	Release()	= 0;
	virtual void	Update()	 {}
	virtual void	LateUpdate() {}
	//virtual void	Render()	= 0;
	
	void			ComponentInit()			override;
	void			ComponentOnEnable()		override;
	void			ComponentUpdate()		override;
	void			ComponentLateUpdate()	override;
	void			ComponentRender()		override;
	void			ComponentOnDisable()	override;
	void			ComponentRelease()		override;

	virtual void	OnCollisionEnter(Collider* other) {};
	virtual void	OnCollisionStay(Collider* other) {};
	virtual void	OnCollisionExit(Collider* other) {};
	
private:
	// Lifetime
	float			lifetime = 0.f;
	float			remainingLifetime = 0.f;
	bool			bHasLifetime = false;
};

