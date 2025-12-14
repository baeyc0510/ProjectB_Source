#include "pch.h"
#include "Hazard.h"

#include "Game/Data/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Util/AnimationHelper.h"

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
	animator = new Animator();
	AddChild(animator);

	// Collider
	collider = new BoxCollider();
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

void Hazard::OnCollisionStay(Collider* other)
{
	if (bIsDestroyed)
		return;

	// 데미지 인터벌 체크
	if (lastDamageTime < damageInterval)
		return;

	if (other->GetLayer() == (UINT)ELayer::Player)
	{
		GameObject* player = other->GetOwner();
		ApplyDamageToPlayer(player);
	}
}

void Hazard::ApplyDamageToPlayer(GameObject* player)
{
	ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
	if (combat && damage > 0.f)
	{
		CombatContext context;
		context.damageType = EDamageType::Normal;
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
	AnimationHelper::AddAnimation(animator,aniName,path,bShouldRepeat);
}
