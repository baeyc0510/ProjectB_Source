#include "pch.h"
#include "Ability_StonerAttack.h"

#include "Game/Component/Rigidbody.h"
#include "Game/Component/StatComponent.h"
#include "Game/Data/AnimKey.h"
#include "Game/Object/Character/Player.h"
#include "Game/Object/Projectile/Projectile_Stone.h"

void Ability_StonerAttack::OnActivate()
{
    Ability::OnActivate();
    
    if (!aiController)
    {
        aiController = owner->GetComponent<AIController>();
    }
    
    GetAnimator()->Play(AnimKey::Attack, true, BIND(this, EndAbility), BIND(this, EndAbility));
    // PlaySFX(SFXKey::);

    WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, ThrowStone));
    WaitEvent(EGameEvent::PlaySFX, [this](GameObject* source)
    {
        //PlaySFX(SFXKey::);
    });
}

void Ability_StonerAttack::OnEnd()
{
    Ability::OnEnd();
    
    ClearEventHandles();
}

void Ability_StonerAttack::ThrowStone()
{
    if (!aiController)
        return;

    Player* player = dynamic_cast<Player*>(aiController->GetTarget());
    if (!player)
        return;
    
    Vec2 targetPos = player->GetCenterPos();
    Vec2 spawnPos = owner->GetPos() + Vec2(owner->GetForward() * STONE_OFFSET_X, STONE_OFFSET_Y);
    
    float dx = targetPos.x - spawnPos.x;
    float dy = targetPos.y - spawnPos.y;

    // 도달 시간 계산 (수평 속도 기준)
    float flightTime = abs(dx) / STONE_SPEED;
    flightTime = max(flightTime, 0.1f);  // 0으로 나누기 방지

    // 실제 적용되는 중력 = GRAVITY_CONSTANT * gravityScale
    constexpr float gravityScale = 0.3f;
    constexpr float gravity = Rigidbody::GRAVITY_CONSTANT * gravityScale;  // 392

    // 중력 보정된 속도 계산
    float vx = dx / flightTime;
    float vy = (dy - 0.5f * gravity * flightTime * flightTime) / flightTime;

    Vec2 velocity(vx, vy);
    
    float damage = 0;
    if (auto stat  = GetStatComponent())
    {
        damage = stat->GetCurrent(EStatType::AttackPower);
    }
    
    // 투사체 생성
    Projectile_Stone* projectile = new Projectile_Stone();
    owner->GetScene()->AddGameObject(projectile);
    projectile->SetPos(spawnPos);
    projectile->SetGravityScale(gravityScale);
    projectile->SetProjectileOwner(owner);
    projectile->SetLifetime(10.f);
    projectile->SetDamage(damage);
    projectile->Launch(velocity);
}
