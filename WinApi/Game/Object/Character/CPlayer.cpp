#include "pch.h"
#include "CPlayer.h"
#include "Game/Ability/Player/Ability_Attack.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAbilitySystem.h"

CPlayer::CPlayer()
	: rigidbody(nullptr)
	, collider(nullptr)
	, speed(300.f)
	, jumpForce(500.f)
	, bIsGrounded(false)
{
	name = TEXT("플레이어");
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	CCharacter::Init();
	
	// Rigidbody
	rigidbody = new CRigidbody();
	AddChild(rigidbody);

	// Collider
	collider = new CCollider();
	collider->SetScale(Vec2(42, 72));
	collider->SetOffset(Vec2(0,-36));
	collider->SetLayer(Layer::Player);
	AddChild(collider);
	
	// Abilities
	AddAbility<Ability_Attack>(EAbility::Attack);
	
	// Animations
	AddAnimation(TEXT("Idle"), TEXT("Animations/Penitent/penitent_idle_anim.json"),true);
	AddAnimation(TEXT("Run"), TEXT("Animations/Penitent/penitent_running_anim.json"),true);
	AddAnimation(TEXT("Jump"), TEXT("Animations/Penitent/penitent_jump_anim.json"),false);
	AddAnimation(TEXT("Fall"), TEXT("Animations/Penitent/penitent_falling_loop.json"),true);
	AddAnimation(TEXT("Combo1"), TEXT("Animations/Penitent/penitent_attack_combo_1.json"),false);
	AddAnimation(TEXT("Combo2"), TEXT("Animations/Penitent/penitent_attack_combo_2.json"),false);
	AddAnimation(TEXT("Combo3"), TEXT("Animations/Penitent/penitent_attack_combo_3.json"),false);
}

void CPlayer::OnEnable()
{
	CCharacter::OnEnable();
	animator->Play(L"Idle", false);
}

void CPlayer::Update()
{
	CCharacter::Update();
	HandleInput();
	UpdateState();
}

void CPlayer::HandleInput()
{
	if (INPUT->ButtonDown('A'))
	{
		abilitySystem->TriggerEvent(EGameEvent::Input_Attack);
		abilitySystem->TryActivateAbility(EAbility::Attack);
	}
	
	// 이동 불가
	if (stateSystem->HasTag(Tag_BlockMovement)) 
	{
		rigidbody->SetVelocity(Vec2(0.0f,0.0f));
		return;
	}

	// 이동
	Vec2 velocity = rigidbody->GetVelocity();
	
	// 이동 입력
	if (INPUT->ButtonStay(VK_LEFT))
	{
		velocity.x = -speed;
		SetForward(-1);
		stateSystem->AddTag(Tag_Moving);
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		velocity.x = speed;
		SetForward(1);
		stateSystem->AddTag(Tag_Moving);
	}
	else
	{
		velocity.x = 0;
		stateSystem->RemoveTag(Tag_Moving);
	}

	// 점프 입력
	if (INPUT->ButtonDown(VK_SPACE) && stateSystem->HasTag(Tag_Grounded))
	{
		velocity.y = -jumpForce;
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTag(Tag_Airborne);
	}

	rigidbody->SetVelocity(velocity);
	animator->SetDirection(GetForward());
}

void CPlayer::UpdateState()
{
	// 착지 체크
	if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
	{
		stateSystem->RemoveTag(Tag_Airborne);
		stateSystem->AddTag(Tag_Grounded);
	}
	if (!bIsGrounded)
	{
		stateSystem->RemoveTag(Tag_Grounded);
		stateSystem->AddTag(Tag_Airborne);
	}

	// Ability가 애니메이션을 제어 중이면 기본 애니메이션 로직 스킵
	if (stateSystem->HasTag(Tag_AbilityAnimation))
		return;

	// 애니메이션 결정
	if (stateSystem->HasTag(Tag_Airborne))
	{
		animator->Play(L"Fall", false);
	}
	else if (stateSystem->HasTag(Tag_Moving))
	{
		animator->Play(L"Run", false);
	}
	else
	{
		animator->Play(L"Idle", false);
	}
}

void CPlayer::Render()
{
	CCharacter::Render();
}

void CPlayer::OnDisable()
{
	CCharacter::OnDisable();
}

void CPlayer::Release()
{
	CCharacter::Release();
}

void CPlayer::OnCollisionEnter(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		// 땅에 처음 닿는 순간 위치 보정
		Vec2 velocity = rigidbody->GetVelocity();
		if (velocity.y > 0) // 아래로 떨어지고 있을 때만
		{
			float playerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
			float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

			float overlap = playerBottom - groundTop;
			if (overlap > 0)
			{
				Vec2 playerPos = GetPos();
				playerPos.y -= overlap;
				SetPos(playerPos);
			}
		}
	}
}

void CPlayer::OnCollisionStay(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		bIsGrounded = true;

		// 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
		float playerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
		float groundTop = other->GetPos().y - other->GetScale().y / 2.f;

		float overlap = playerBottom - groundTop;
		if (overlap > 0)
		{
			Vec2 playerPos = GetPos();
			playerPos.y -= overlap;
			SetPos(playerPos);
		}

		// 땅을 뚫고 올라가는 것을 방지
		Vec2 velocity = rigidbody->GetVelocity();
		if (velocity.y > 0)
		{
			velocity.y = 0.f;
			rigidbody->SetVelocity(velocity);
		}
	}
}

void CPlayer::OnCollisionExit(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		bIsGrounded = false;
	}
}