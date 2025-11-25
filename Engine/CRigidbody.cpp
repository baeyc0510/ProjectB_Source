#include "pch.h"
#include "CRigidbody.h"

CRigidbody::CRigidbody()
    : m_vVelocity(Vec2(0.f, 0.f))
    , m_fGravityScale(1.f)
    , m_bUseGravity(true)
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
    if (m_bUseGravity)
    {
        m_vVelocity.y += GRAVITY_CONSTANT * m_fGravityScale * DT;
    }

    // Update owner's position
    Vec2 vPos = GetOwner()->GetPos();
    vPos += m_vVelocity * DT;
    GetOwner()->SetPos(vPos);
}

void CRigidbody::ComponentRelease()
{
}