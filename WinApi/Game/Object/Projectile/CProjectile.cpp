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
}

void CProjectile::Update()
{
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
	if (IsReservedDelete())
		return;

	UINT otherLayer = other->GetLayer();
	// 땅과 충돌
	if (otherLayer == ELayer::Ground)
	{
		OnHitGround(GetPos());
	}
	// 벽과 충돌 (플랫폼 측면 등)
	// Ground와 동일하게 처리하되 별도의 이벤트 호출
}

void CProjectile::OnCollisionStay(CCollider* other)
{
	UINT otherLayer = other->GetLayer();
	// 플레이어와 충돌
	if (otherLayer == ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		OnHitPlayer(player);
	}
}

void CProjectile::OnDamage(CGameObject* source, const CombatContext& context)
{
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
}

void CProjectile::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
	assert(animator);
	CAnimation* animation = LOADANIMATION(name + L"_" + aniName, path);
	assert(animation);
	animation->SetRepeat(bShouldRepeat);
	animator->AddAnimation(aniName, animation);
}
