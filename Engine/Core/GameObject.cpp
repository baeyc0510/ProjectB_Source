#include "pch.h"
#include "GameObject.h"

GameObject::GameObject()
{
	name	= TEXT("");
	pos		= Vec2(0, 0);
	scale	= Vec2(0, 0);
	forward = 1;
	zOrder	= 0;
}

GameObject::~GameObject()
{
}

void GameObject::ComponentInit()
{
	Init();
	Composite::ComponentInit();
}

void GameObject::ComponentOnEnable()
{
	OnEnable();
	Composite::ComponentOnEnable();
}

void GameObject::ComponentUpdate()
{
	if (IsReservedDelete())
		return;

	// Lifetime 처리: 시간이 다 되면 Destroy 호출
	if (bHasLifetime)
	{
		remainingLifetime -= DT;
		if (remainingLifetime <= 0.f)
		{
			Destroy();
			return;
		}
	}

	// 부모 게임오브젝트가 있는 경우, 위치는 부모를 기준으로한 상대위치
	if (GetOwner() != nullptr)
		worldPos = GetOwner()->GetWorldPos() + pos;
	// 부모 게임오브젝트가 없는 경우, 위치는 월드를 기준으로한 절대위치
	else
		worldPos = pos;

	renderPos = SINGLE(CameraManager)->WorldToScreenPoint(worldPos);

	Update();
	Composite::ComponentUpdate();
}

void GameObject::ComponentLateUpdate()
{
	if (IsReservedDelete())
		return;
	
	LateUpdate();
	Composite::ComponentLateUpdate();
}

void GameObject::ComponentRender()
{
	if (IsReservedDelete())
		return;

	GetScene()->AddRenderer(this);
	Composite::ComponentRender();
}

void GameObject::ComponentOnDisable()
{
	Composite::ComponentOnDisable();
	OnDisable();
}

void GameObject::ComponentRelease()
{
	Composite::ComponentRelease();
	Release();
}

void GameObject::SetLifetime(float seconds)
{
	lifetime = seconds;
	remainingLifetime = seconds;
	bHasLifetime = true;
}

void GameObject::Destroy()
{
	if (IsReservedDelete())
		return;

	OnDestroy();
	WORLD->Delete(GetScene(), this);
}
