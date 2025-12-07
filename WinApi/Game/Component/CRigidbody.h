#pragma once
#include "Core/CGameObject.h"
#include "Core/Component.h"
#include "Util/Struct.h"

class CRigidbody : public Component<CGameObject>
{
public:
    CRigidbody();
    virtual ~CRigidbody();

    /*~ CRigidbody Interface ~*/
    Vec2 GetVelocity() { return velocity; }
    bool IsUsingGravity() { return bUseGravity; }
    float GetGravityScale() { return gravityScale; }

    void SetVelocity(Vec2 inVelocity) { velocity = inVelocity; }
    void SetVelocity(float x, float y) { velocity = Vec2(x, y); }
    void UseGravity(bool use) { bUseGravity = use; }
    void SetGravityScale(float scale) { gravityScale = scale; }
    void SetGrounded(bool grounded) { bGrounded = grounded; }
    void AddVelocity(Vec2 inVelocity) { velocity += inVelocity; }

protected:
    /*~ Component Interface ~*/
	void ComponentInit() override;
	void ComponentUpdate() override;
	void ComponentRender() override;
	void ComponentRelease() override;

private:
    Vec2 velocity;
    float gravityScale;
    bool bUseGravity;
    bool bGrounded;

    static constexpr float GRAVITY_CONSTANT = 980.f;
};
