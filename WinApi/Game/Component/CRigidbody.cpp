#include "pch.h"
#include "CRigidbody.h"
#include "Managers/CTimeManager.h"
#include "Util/Struct.h"

CRigidbody::CRigidbody()
    : velocity(Vec2(0.f, 0.f))
    , gravityScale(1.f)
    , bUseGravity(true)
{
}

CRigidbody::~CRigidbody()
{
}


void CRigidbody::ComponentInit()
{
}

void CRigidbody::ComponentRender()
{
}

void CRigidbody::ComponentUpdate()
{
    // Apply gravity
    if (bUseGravity)
    {
        velocity.y += GRAVITY_CONSTANT * gravityScale * DT;
    }

    // Update owner's position
    Vec2 vPos = GetOwner()->GetPos();
    vPos += velocity * DT;
    GetOwner()->SetPos(vPos);
}

void CRigidbody::ComponentRelease()
{
}