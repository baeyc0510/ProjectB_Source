#include "pch.h"
#include "Projectile.h"

#include "Game/Component/Rigidbody.h"
#include "Game/VFXKeys.h"
#include "Game/Manager/VFXManager.h"
#include "Game/Object/VFXObject.h"

Projectile::Projectile()
{
	name = TEXT("Projectile");
}

Projectile::~Projectile()
{
}

void Projectile::Init()
{
	// Animator
	animator = new Animator();
	AddChild(animator);

	// Rigidbody
	rigidbody = new Rigidbody();
	rigidbody->UseGravity(true);
	AddChild(rigidbody);

	// Collider
	collider = new BoxCollider();
	collider->SetScale(Vec2(20, 20));
	collider->SetLayer((UINT)ELayer::Projectile);
	AddChild(collider);
}

void Projectile::OnEnable()
{
}

void Projectile::Update()
{
}

void Projectile::Render()
{
}

void Projectile::OnDisable()
{
}

void Projectile::Release()
{
}

void Projectile::SetVelocity(Vec2 vel)
{
	if (rigidbody)
	{
		rigidbody->SetVelocity(vel);
	}
}

void Projectile::Launch(Vec2 velocity)
{
	SetVelocity(velocity);
	SetForward(velocity.x > 0 ? 1 : -1);
	if (animator)
	{
		animator->SetDirection(GetForward());
	}
}

void Projectile::OnCollisionEnter(Collider* other)
{
	if (IsReservedDelete())
		return;

	UINT otherLayer = other->GetLayer();
	// 땅과 충돌
	if (otherLayer == (UINT)ELayer::Ground)
	{
		OnHitGround(GetPos());
	}
	// 벽과 충돌 (플랫폼 측면 등)
	// Ground와 동일하게 처리하되 별도의 이벤트 호출
}

void Projectile::OnCollisionStay(Collider* other)
{
	UINT otherLayer = other->GetLayer();
	// 플레이어와 충돌
	if (otherLayer == (UINT)ELayer::Player)
	{
		GameObject* player = other->GetOwner();
		OnHitPlayer(player);
	}
}

void Projectile::OnDamage(GameObject* source, const CombatContext& context)
{
}

void Projectile::OnHitGround(Vec2 hitPos)
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void Projectile::OnHitWall(Vec2 hitPos)
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}

void Projectile::OnHitPlayer(GameObject* player)
{
}

void Projectile::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
	assert(animator);
	AnimationResource* animation = LOADANIMATION(name + L"_" + aniName, path);
	assert(animation);
	animation->SetRepeat(bShouldRepeat);
	animator->AddAnimation(aniName, animation);
}
