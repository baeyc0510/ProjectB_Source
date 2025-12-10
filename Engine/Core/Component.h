#pragma once

// 디자인패턴 컴포지트 :
// 여러 개의 객체들로 구성된 복합 객체와 단일 객체를
// 클라이언트에서 구별 없이 다루게 해주는 패턴
// 전체-부분의 관계를 갖는 객체들 사이의 관계를 정의할 때 유용
// 트리형 구조를 가지게 됨
// 
// 게임프로그래밍 컴포넌트 : 
// 프로그래밍에 있어 기능 재사용이 가능한 각각의 독립된 모듈
// 컴포지트 구조에서 부모에 포함될 경우 독립적으로 동작하는 자식 모듈로 사용
// 상속의 경우 모든 상속하는 자식에게 반드시 기능이 포함되지만
// 컴포넌트의 경우 필요한 객체에만 조립식으로 붙일 수 있음

class WorldManager;
template <typename T>
class Composite;
class Scene;

template <typename T>
class Component
{
	friend WorldManager;
	friend Composite<T>;
public:
	Component() {}
	virtual ~Component() {}

public:
	// 선택적 오버라이드 - 자식 클래스에서 필요한 경우에만 구현
	virtual void	ComponentInit()			{}
	virtual void	ComponentOnEnable()		{ active = true; }
	virtual void	ComponentUpdate()		{}
	virtual void	ComponentLateUpdate()	{}
	virtual void	ComponentRender()		{}
	virtual void	ComponentOnDisable()	{ active = false; }
	virtual void	ComponentRelease()		{}

public:
	T*				GetOwner()				{ return owner; }
	bool			IsActive()				{ return active; }
	Scene*			GetScene()				{ return (owner == nullptr) ? scene : owner->GetScene(); }
	bool			IsReservedDelete()		{ return reservedDelete; }

protected:
	void			SetOwner(T* owner)		{ this->owner = owner; }
	virtual void			SetActive(bool active)	{ this->active = active; }
	void			SetScene(Scene* scene) { this->scene = scene; }
	virtual void	SetReservedDelete()		{ reservedDelete = true; }	// 컴포넌트 삭제 예약

	virtual void	DeleteReservedChild()	{}

protected:
	T*				owner					= nullptr;
	bool			active					= false;
	Scene*			scene					= nullptr;
	bool			reservedDelete			= false;					// 컴포넌트가 삭제예정인지 여부
};

template <typename T>
class Composite abstract : public Component<T>
{
public:
	Composite() {}
	virtual ~Composite() {}

public:
	void ComponentInit() override
	{
	}

	void ComponentOnEnable() override
	{
		this->active = true;
		for (Component<T>* component : childList)
		{
			component->ComponentOnEnable();
		}
	}

	void ComponentUpdate() override
	{
		for (Component<T>* component : childList)
		{
			component->ComponentUpdate();
		}
	}

	void ComponentLateUpdate() override
	{
		for (Component<T>* component : childList)
		{
			component->ComponentLateUpdate();
		}
	}

	void ComponentRender() override
	{
		for (Component<T>* component : childList)
		{
			component->ComponentRender();
		}
	}

	void ComponentOnDisable() override
	{
		for (Component<T>* component : childList)
		{
			component->ComponentOnDisable();
		}
		this->active = false;
	}

	void ComponentRelease() override
	{
		for (Component<T>* component : childList)
		{
			component->ComponentRelease();
			delete component;
		}
		childList.clear();
	}

	void AddChild(Component<T>* component)
	{
		childList.push_back(component);
		component->SetOwner((T*)this);
		component->ComponentInit();
		if (this->IsActive()) component->ComponentOnEnable();
	}

	void RemoveChild(Component<T>* component)
	{
		if (this->IsActive()) component->ComponentOnDisable();
		component->ComponentRelease();
		component->SetOwner(nullptr);
		childList.remove(component);
		delete component;
	}

	template <typename ComponentType>
	ComponentType* GetComponent()
	{
		for (Component<T>* component : childList)
		{
			ComponentType* casted = dynamic_cast<ComponentType*>(component);
			if (casted)
				return casted;
		}
		return nullptr;
	}
	
	template <typename ComponentType>
	ComponentType* AddOrGetComponent()
	{
		ComponentType* component = GetComponent<ComponentType>();
		if (!component)
		{
			component = new ComponentType();
			AddChild(component);
		}
		return component;
	}

protected:
	void SetReservedDelete() override
	{
		this->reservedDelete = true;
		for (Component<T>* component : childList)
		{
			component->SetReservedDelete();
		}
	}

	void DeleteReservedChild() override
	{
		childList.remove_if([&](Component<T>* child) {
			if (child->IsReservedDelete())
			{
				child->SetOwner(nullptr);
				if (this->IsActive()) child->ComponentOnDisable();
				child->ComponentRelease();
				delete child;
				return true;
			}
			else
			{
				child->DeleteReservedChild();
				return false;
			}
			});
	}

protected:
	list<Component<T>*> childList;
};