#include "pch.h"
#include "CProjectile.h"

#include "Game/Component/CRigidbody.h"
#include "Game/VFXKeys.h"
#include "Game/Manager/CVFXManager.h"
#include "Game/Object/CVFX.h"

CProjectile::CProjectile()
{
	name = TEXT("Projectile");
}

CProjectile::~CProjectile()
{
}

void CProjectile::Init()
{
	// Animator
	animator = new CAnimator();
	AddChild(animator);

	// Rigidbody
	rigidbody = new CRigidbody();
	rigidbody->UseGravity(true);
	AddChild(rigidbody);

	// Collider
	collider = new CBoxCollider();
	collider->SetScale(Vec2(20, 20));
	collider->SetLayer(ELayer::Projectile);
	AddChild(collider);
}

void CProjectile::OnEnable()
{
	remainingLifetime = lifetime;
	bIsDestroyed = false;
}

void CProjectile::Update()
{
	if (bIsDestroyed)
		return;

	// 수명 체크
	if (bHasLifetime)
	{
		remainingLifetime -= DT;
		if (remainingLifetime <= 0.f)
		{
			DestroySelf();
			return;
		}
	}
}

void CProjectile::Render()
{
}

void CProjectile::OnDisable()
{
}

void CProjectile::Release()
{
}

void CProjectile::SetVelocity(Vec2 vel)
{
	if (rigidbody)
	{
		rigidbody->SetVelocity(vel);
	}
}

void CProjectile::Launch(Vec2 velocity)
{
	SetVelocity(velocity);
	SetForward(velocity.x > 0 ? 1 : -1);
	if (animator)
	{
		animator->SetDirection(GetForward());
	}
}

void CProjectile::OnCollisionEnter(CCollider* other)
{
	if (bIsDestroyed)
		return;

	UINT otherLayer = other->GetLayer();

	// 플레이어와 충돌
	if (otherLayer == ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		OnHitPlayer(player);
		DestroySelf();
	}
	// 땅과 충돌
	else if (otherLayer == ELayer::Ground)
	{
		OnHitGround(GetPos());
		DestroySelf();
	}
	// 벽과 충돌 (플랫폼 측면 등)
	// Ground와 동일하게 처리하되 별도의 이벤트 호출
}

void CProjectile::OnDamage(CGameObject* source, const CombatContext& context)
{
	// 플레이어 공격에 의해 파괴됨
	if (bIsDestroyed)
		return;

	// VFX 생성
	if (!context.vfxKey.empty())
	{
		if (CVFX* vfx = VFX->CreateVFX(context.vfxKey, GetPos(), source->GetForward()))
		{
			vfx->PlayVFX();
		}
	}

	OnDestroyed();
	DestroySelf();
}

void CProjectile::OnHitGround(Vec2 hitPos)
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void CProjectile::OnHitWall(Vec2 hitPos)
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void CProjectile::OnHitPlayer(CGameObject* player)
{
	// 플레이어에게 데미지
	ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
	if (combat && damage > 0.f)
	{
		CombatContext context;
		context.damageType = EDamageType::Slash;
		context.value = damage;
		context.vfxKey = VFXKey::AttackHit1;
		combat->OnDamage(projectileOwner, context);
	}
}

void CProjectile::OnDestroyed()
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void CProjectile::DestroySelf()
{
	if (bIsDestroyed)
		return;

	bIsDestroyed = true;
	WORLD->Delete(GetScene(), this);
}
