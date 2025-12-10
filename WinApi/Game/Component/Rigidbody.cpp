#include "pch.h"
#include "Rigidbody.h"
#include "Managers/TimeManager.h"
#include "Util/Struct.h"

Rigidbody::Rigidbody()
    : velocity(Vec2(0.f, 0.f))
    , gravityScale(1.f)
    , bUseGravity(true)
    , bGrounded(false)
{
}

Rigidbody::~Rigidbody()
{
}


void Rigidbody::ComponentInit()
{
}

void Rigidbody::ComponentRender()
{
}

void Rigidbody::ComponentUpdate()
{
    // Apply gravity (땅에 있지 않을 때만)
    if (bUseGravity && !bGrounded)
    {
        velocity.y += GRAVITY_CONSTANT * gravityScale * DT;
    }

    // Update position
    Vec2 vPos = GetOwner()->GetPos();
    vPos.x += velocity.x * DT;

    // 땅에 있을 때는 하강은 무시
    if (!bGrounded || velocity.y < 0)
    {
        vPos.y += velocity.y * DT;
    }

    GetOwner()->SetPos(vPos);
}

void Rigidbody::ComponentRelease()
{
}