#include "pch.h"
#include "Projectile_Stone.h"

#include "Game/Data/AnimKey.h"
#include "Game/Data/VFXKeys.h"
#include "Game/Object/Character/Character.h"

void Projectile_Stone::Init()
{
    Projectile::Init();
    
    // rigidbody
    SetGravityScale(0.4f);
    
    // animation
    AddAnimation(AnimKey::Idle, TEXT("Animations/Projectile/thrown_rock_anim.json"), true);
}

void Projectile_Stone::Update()
{
    Projectile::Update();
    animator->Play(AnimKey::Idle);
}

void Projectile_Stone::OnHitPlayer(GameObject* player)
{
    Projectile::OnHitPlayer(player);
    
    ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
    if (!combat)
        return;

    CombatContext context;
    context.damageType = EDamageType::Normal;
    context.value = damage;
    context.vfxKey = VFXKey::PlayerHit;
    
    combat->OnDamage(projectileOwner,context);
    Destroy();
}

void Projectile_Stone::OnHitGround(Vec2 hitPos, Collider* groundCol)
{
    if (Character* character = dynamic_cast<Character*>(projectileOwner))
    {
        if (groundCol->GetID() == character->GetCurrentGroundID())
        {
            return;
        }
    }
    
    Destroy();
}