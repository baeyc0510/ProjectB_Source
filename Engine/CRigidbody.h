#pragma once
#include "Component.h"

class CRigidbody : public Component<CGameObject>
{
public:
    CRigidbody();
    virtual ~CRigidbody();

	void ComponentInit() override;
	void ComponentRender() override;
    void ComponentUpdate() override;
	void ComponentRelease() override;

public:
    // Getters
    Vec2    GetVelocity() { return m_vVelocity; }
    bool    IsUsingGravity() { return m_bUseGravity; }
    float   GetGravityScale() { return m_fGravityScale; }

    // Setters
    void    SetVelocity(Vec2 velocity) { m_vVelocity = velocity; }
    void    SetVelocity(float x, float y) { m_vVelocity = Vec2(x, y); }
    void    UseGravity(bool use) { m_bUseGravity = use; }
    void    SetGravityScale(float scale) { m_fGravityScale = scale; }

private:
    Vec2    m_vVelocity;
    float   m_fGravityScale;
    bool    m_bUseGravity;

    // A world-wide gravity constant. Could be moved to a PhysicsManager later.
    const float GRAVITY_CONSTANT = 980.f; 
};