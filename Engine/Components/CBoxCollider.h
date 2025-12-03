#pragma once
#include "CCollider.h"

class CBoxCollider : public CCollider
{
public:
	CBoxCollider();
	virtual ~CBoxCollider();

public:
	Vec2				GetScale() const override		{ return scale; }
	void				SetScale(const Vec2& scale) override { this->scale = scale; }

	EColliderType		GetType() const override		{ return EColliderType::Box; }

private:
	void				Render() override;

	Vec2				scale;
};
