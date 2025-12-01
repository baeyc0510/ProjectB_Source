#pragma once
#include "Core/CGameObject.h"
#include "Core/Component.h"
#include "Util/Struct.h"

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
    Vec2 GetVelocity() { return velocity; }
    bool IsUsingGravity() { return bUseGravity; }
    float GetGravityScale() { return gravityScale; }

    // Setters
    void SetVelocity(Vec2 inVelocity) { velocity = inVelocity; }
    void SetVelocity(float x, float y) { velocity = Vec2(x, y); }
    void UseGravity(bool use) { bUseGravity = use; }
    void SetGravityScale(float scale) { gravityScale = scale; }
    void SetGrounded(bool grounded) { bGrounded = grounded; }

    void AddVelocity(Vec2 inVelocity) { velocity += inVelocity; }
private:
    Vec2 velocity;
    float gravityScale;
    bool bUseGravity;
    bool bGrounded;

    // A world-wide gravity constant. Could be moved to a PhysicsManager later.
    const float GRAVITY_CONSTANT = 980.f; 
};
