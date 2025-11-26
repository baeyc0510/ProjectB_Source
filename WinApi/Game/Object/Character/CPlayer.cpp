#include "pch.h"
#include "CPlayer.h"

#include "Game/Component/CRigidbody.h"

CPlayer::CPlayer()
	: rigidbody(nullptr)
	, animator(nullptr)
	, collider(nullptr)
	, speed(300.f)
	, jumpForce(500.f)
	, state(PlayerState::Idle)
	, bIsGrounded(false)
	, isMove(false)
	, direction(1)
{
	name = TEXT("플레이어");
	scale = Vec2(100, 100);
	moveDir = Vec2(0, 0);
	lookDir = Vec2(0, -1);
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	// Rigidbody
	rigidbody = new CRigidbody();
	AddChild(rigidbody);

	// 애니메이션
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

	// Collider
	collider = new CCollider();
	collider->SetScale(Vec2(90, 90));
	collider->SetLayer(Layer::Player);
	AddChild(collider);
}

void CPlayer::OnEnable()
{
}

void CPlayer::Update()
{
	// 1. 상태에 따른 행동 처리 (State-specific Actions)
	Vec2 vVelocity = rigidbody->GetVelocity();

	isMove = false;
	if (INPUT->ButtonStay(VK_LEFT))
	{
		vVelocity.x = -speed;
		isMove = true;
		direction = -1;
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		vVelocity.x = speed;
		isMove = true;
		direction = 1;
	}
	else
	{
		vVelocity.x = 0;
	}

	rigidbody->SetVelocity(vVelocity);
	animator->SetDirection(direction);

	// 2. 상태 전환 로직 (State Transitions)
 	switch (state)
	{
	case PlayerState::Idle:
	case PlayerState::Run:
		if (bIsGrounded && INPUT->ButtonDown(VK_SPACE))
		{
			Vec2 vel = rigidbody->GetVelocity();
			vel.y = -jumpForce;
			rigidbody->SetVelocity(vel);
			state = PlayerState::Jump;
			bIsGrounded = false;
		}
		else if (!bIsGrounded)
		{
			state = PlayerState::Fall;
		}
		else if (isMove)
		{
			state = PlayerState::Run;
		}
		else
		{
			state = PlayerState::Idle;
		}
		break;

	case PlayerState::Jump:
		if (animator->IsFinished())
		{
			state = PlayerState::Fall;
		}
		break;

	case PlayerState::Fall:
		if (bIsGrounded)
		{
			state = PlayerState::Idle;
		}
		break;
	}

	// 3. 애니메이터 업데이트
	AnimatorUpdate();

	// 4. Grounded 플래그 매 프레임 초기화
	bIsGrounded = false;
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
		Vec2 vVelocity = rigidbody->GetVelocity();
		if (vVelocity.y > 0) // 아래로 떨어지고 있을 때만
		{
			float fPlayerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
			float fGroundTop = other->GetPos().y - other->GetScale().y / 2.f;

			float fOverlap = fPlayerBottom - fGroundTop;
			if (fOverlap > 0)
			{
				Vec2 vPlayerPos = GetPos();
				vPlayerPos.y -= fOverlap;
				SetPos(vPlayerPos);
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
		float fPlayerBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
		float fGroundTop = other->GetPos().y - other->GetScale().y / 2.f;

		float fOverlap = fPlayerBottom - fGroundTop;
		if (fOverlap > 0)
		{
			Vec2 vPlayerPos = GetPos();
			vPlayerPos.y -= fOverlap;
			SetPos(vPlayerPos);
		}

		// 땅을 뚫고 올라가는 것을 방지
		Vec2 vVelocity = rigidbody->GetVelocity();
		if (vVelocity.y > 0)
		{
			vVelocity.y = 0.f;
			rigidbody->SetVelocity(vVelocity);
		}
	}
}

void CPlayer::OnCollisionExit(CCollider* other)
{
	if (other->GetLayer() == Layer::Ground)
	{
		// m_bIsGrounded = false; // Update 시작 시 초기화하므로 여기서는 필요 없음
	}
}

void CPlayer::AnimatorUpdate()
{
	switch (state)
	{
	case PlayerState::Idle:
		animator->Play(L"Idle", false);
		break;
	case PlayerState::Run:
		animator->Play(L"Run", false);
		break;
	case PlayerState::Jump:
		animator->Play(L"Jump", false);
		break;
	case PlayerState::Fall:
		animator->Play(L"Fall", false);
		break;
	}
}
