#include "pch.h"
#include "Hazard.h"

#include "Game/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"

Hazard::Hazard()
{
	name = TEXT("Hazard");
}

Hazard::~Hazard()
{
}

void Hazard::Init()
{
	// Animator
	animator = new CAnimator();
	AddChild(animator);

	// Collider
	collider = new CBoxCollider();
	collider->SetScale(Vec2(30, 50));
	collider->SetLayer((UINT)ELayer::Hazard);
	AddChild(collider);
}

void Hazard::OnEnable()
{
	lastDamageTime = 0.f;
	bIsDestroyed = false;
	OnSpawn();
}

void Hazard::Update()
{
	// 데미지 인터벌 타이머
	lastDamageTime += DT;
}

void Hazard::Render()
{
}

void Hazard::OnDisable()
{
}

void Hazard::Release()
{
}

void Hazard::OnCollisionEnter(CCollider* other)
{
	if (bIsDestroyed)
		return;

	if (other->GetLayer() == (UINT)ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		DealDamageToPlayer(player);
	}
}

void Hazard::OnCollisionStay(CCollider* other)
{
	if (bIsDestroyed)
		return;

	// 데미지 인터벌 체크
	if (lastDamageTime < damageInterval)
		return;

	if (other->GetLayer() == (UINT)ELayer::Player)
	{
		CGameObject* player = other->GetOwner();
		DealDamageToPlayer(player);
	}
}

void Hazard::DealDamageToPlayer(CGameObject* player)
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

void Hazard::OnSpawn()
{
	// 기본 구현: 없음 (파생 클래스에서 오버라이드)
}
void Hazard::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
	assert(animator);
	CAnimation* animation = LOADANIMATION(name + L"_" + aniName, path);
	assert(animation);
	animation->SetRepeat(bShouldRepeat);
	animator->AddAnimation(aniName, animation);
}
