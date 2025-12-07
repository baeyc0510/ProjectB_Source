#include "pch.h"
#include "CHazard.h"

#include "Game/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"

CHazard::CHazard()
{
	name = TEXT("Hazard");
}

CHazard::~CHazard()
{
}

void CHazard::Init()
{
	// Animator
	animator = new CAnimator();
	AddChild(animator);

	// Collider
	collider = new CBoxCollider();
	collider->SetScale(Vec2(30, 50));
	collider->SetLayer(ELayer::Hazard);
	AddChild(collider);
}

void CHazard::OnEnable()
{
	remainingLifetime = lifetime;
	lastDamageTime = 0.f;
	bIsDestroyed = false;
	OnSpawn();
}

void CHazard::Update()
{
	if (bIsDestroyed)
		return;

	// 수명 체크
	if (bHasLifetime)
	{
		remainingLifetime -= DT;
		if (remainingLifetime <= 0.f)
		{
			OnExpire();
			DestroySelf();
			return;
		}
	}

	// 데미지 인터벌 타이머
	lastDamageTime += DT;
}

void CHazard::Render()
{
}

void CHazard::OnDisable()
{
}

void CHazard::Release()
{
}

void CHazard::OnCollisionEnter(CCollider* other)
{
	if (bIsDestroyed)
		return;

	if (other->GetLayer() == ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		DealDamageToPlayer(player);
	}
}

void CHazard::OnCollisionStay(CCollider* other)
{
	if (bIsDestroyed)
		return;

	// 데미지 인터벌 체크
	if (lastDamageTime < damageInterval)
		return;

	if (other->GetLayer() == ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		DealDamageToPlayer(player);
	}
}

void CHazard::DealDamageToPlayer(CGameObject* player)
{
	ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
	if (combat && damage > 0.f)
	{
		CombatContext context;
		context.damageType = EDamageType::Slash;
		context.value = damage;
		context.vfxKey = VFXKey::PlayerHit;
		combat->OnDamage(this, context);

		lastDamageTime = 0.f;
	}
}

void CHazard::OnSpawn()
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void CHazard::OnExpire()
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void CHazard::DestroySelf()
{
	if (bIsDestroyed)
		return;

	bIsDestroyed = true;
	WORLD->Delete(GetScene(), this);
}
