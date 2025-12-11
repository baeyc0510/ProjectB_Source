#pragma once
#include "Core/GameObject.h"
#include "Core/Component.h"
#include "Util/Struct.h"

class Rigidbody : public Component<GameObject>
{
public:
    Rigidbody();
    virtual ~Rigidbody();

    /*~ Rigidbody Interface ~*/
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

public:
    static constexpr float GRAVITY_CONSTANT = 980.f;
    
private:
    Vec2 velocity;
    float gravityScale;
    bool bUseGravity;
    bool bGrounded;
};
