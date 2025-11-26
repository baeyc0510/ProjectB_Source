#include "pch.h"
#include "CPlayer.h"

#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Util/AnimEventHelper.h"

CPlayer::CPlayer()
	: rigidbody(nullptr)
	, animator(nullptr)
	, collider(nullptr)
	, stateSystem(nullptr)
	, abilitySystem(nullptr)
	, speed(300.f)
	, jumpForce(500.f)
	, direction(1)
	, bIsGrounded(false)
{
	name = TEXT("플레이어");
	scale = Vec2(100, 100);
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	// Rigidbody
	rigidbody = new CRigidbody();
	AddChild(rigidbody);

	// Collider
	collider = new CCollider();
	collider->SetScale(Vec2(90, 90));
	collider->SetLayer(Layer::Player);
	AddChild(collider);

	// StateSystem
	stateSystem = new CStateSystem();
	AddChild(stateSystem);
	stateSystem->AddTag(Tag_Grounded);

	// AbilitySystem
	abilitySystem = new CAbilitySystem();
	AddChild(abilitySystem);
	
	// Animator
	CAnimation* idleAnimation = LOADANIMATION(TEXT("PlayerIdle"), TEXT("Animations/Player_Idle.json"));
	idleAnimation->SetRepeat(true);

	CAnimation* runAnimation = LOADANIMATION(TEXT("PlayerRun"), TEXT("Animations/Player_Run.json"));
	runAnimation->SetRepeat(true);

	CAnimation* jumpAnimation = LOADANIMATION(TEXT("PlayerJump"), TEXT("Animations/Player_StartJump.json"));
	jumpAnimation->SetRepeat(false);

	CAnimation* fallAnimation = LOADANIMATION(TEXT("PlayerFall"), TEXT("Animations/Player_Fall.json"));
	fallAnimation->SetRepeat(true);

	animator = new CAnimator();
	animator->AddAnimation(TEXT("Idle"), idleAnimation);
	animator->AddAnimation(TEXT("Run"), runAnimation);
	animator->AddAnimation(TEXT("Jump"), jumpAnimation);
	animator->AddAnimation(TEXT("Fall"), fallAnimation);
	AddChild(animator);
	
	// Animator -> AbilitySystem 이벤트 연결
	AnimEventHelper::ConnectAbilitySystem(animator, abilitySystem);
}

void CPlayer::OnEnable()
{
}

void CPlayer::Update()
{
	HandleInput();
	UpdateState();

	// Grounded 플래그 매 프레임 초기화
	bIsGrounded = false;
}

void CPlayer::HandleInput()
{
	Vec2 velocity = rigidbody->GetVelocity();

	// 이동 입력
	if (INPUT->ButtonStay(VK_LEFT))
	{
		velocity.x = -speed;
		direction = -1;
		stateSystem->AddTag(Tag_Moving);
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		velocity.x = speed;
		direction = 1;
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
	animator->SetDirection(direction);
}

void CPlayer::UpdateState()
{
	// 착지 체크
	if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
	{
		stateSystem->RemoveTag(Tag_Airborne);
		stateSystem->AddTag(Tag_Grounded);
	}

	// Ability가 애니메이션을 제어 중이면 기본 애니메이션 로직 스킵
	if (stateSystem->HasTag(Tag_AbilityPlaying))
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
}

void CPlayer::OnDisable()
{
}

void CPlayer::Release()
{
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
			Vec2 vPlayerPos = GetPos();
			vPlayerPos.y -= overlap;
			SetPos(vPlayerPos);
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
}
