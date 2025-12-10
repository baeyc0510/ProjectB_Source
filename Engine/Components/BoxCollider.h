#pragma once
#include "Collider.h"

class BoxCollider : public Collider
{
public:
	BoxCollider();
	virtual ~BoxCollider();

public:
	Vec2				GetScale() const override		{ return scale; }
	void				SetScale(const Vec2& scale) override { this->scale = scale; }

	EColliderType		GetType() const override		{ return EColliderType::Box; }

	void RenderDebug() override;
private:
	void				Render() override;

	Vec2				scale;
};
