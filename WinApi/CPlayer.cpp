#include "pch.h"
#include "CPlayer.h"
#include "CRigidbody.h"
#include "CAnimator.h"
#include "CAnimation.h"
#include "CCollider.h"

CPlayer::CPlayer()
	: m_pRigidbody(nullptr)
	, animator(nullptr)
	, m_pCollider(nullptr)
	, speed(300.f)
	, m_fJumpForce(500.f)
	, m_eState(PlayerState::Idle)
	, m_bIsGrounded(false)
	, isMove(false)
	, m_iDirection(1)
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
	m_pRigidbody = new CRigidbody();
	AddChild(m_pRigidbody);

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
	m_pCollider = new CCollider();
	m_pCollider->SetScale(Vec2(90, 90));
	m_pCollider->SetLayer(Layer::Player);
	AddChild(m_pCollider);
}

void CPlayer::OnEnable()
{
}

void CPlayer::Update()
{
	// 1. 상태에 따른 행동 처리 (State-specific Actions)
	Vec2 vVelocity = m_pRigidbody->GetVelocity();

	isMove = false;
	if (INPUT->ButtonStay(VK_LEFT))
	{
		vVelocity.x = -speed;
		isMove = true;
		m_iDirection = -1;
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		vVelocity.x = speed;
		isMove = true;
		m_iDirection = 1;
	}
	else
	{
		vVelocity.x = 0;
	}

	m_pRigidbody->SetVelocity(vVelocity);
	animator->SetDirection(m_iDirection);

	// 2. 상태 전환 로직 (State Transitions)
 	switch (m_eState)
	{
	case PlayerState::Idle:
	case PlayerState::Run:
		if (m_bIsGrounded && INPUT->ButtonDown(VK_SPACE))
		{
			Vec2 vel = m_pRigidbody->GetVelocity();
			vel.y = -m_fJumpForce;
			m_pRigidbody->SetVelocity(vel);
			m_eState = PlayerState::Jump;
			m_bIsGrounded = false;
		}
		else if (!m_bIsGrounded)
		{
			m_eState = PlayerState::Fall;
		}
		else if (isMove)
		{
			m_eState = PlayerState::Run;
		}
		else
		{
			m_eState = PlayerState::Idle;
		}
		break;

	case PlayerState::Jump:
		if (animator->IsFinished())
		{
			m_eState = PlayerState::Fall;
		}
		break;

	case PlayerState::Fall:
		if (m_bIsGrounded)
		{
			m_eState = PlayerState::Idle;
		}
		break;
	}

	// 3. 애니메이터 업데이트
	AnimatorUpdate();

	// 4. Grounded 플래그 매 프레임 초기화
	m_bIsGrounded = false;
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
		Vec2 vVelocity = m_pRigidbody->GetVelocity();
		if (vVelocity.y > 0) // 아래로 떨어지고 있을 때만
		{
			float fPlayerBottom = m_pCollider->GetPos().y + m_pCollider->GetScale().y / 2.f;
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
		m_bIsGrounded = true;

		// 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
		float fPlayerBottom = m_pCollider->GetPos().y + m_pCollider->GetScale().y / 2.f;
		float fGroundTop = other->GetPos().y - other->GetScale().y / 2.f;

		float fOverlap = fPlayerBottom - fGroundTop;
		if (fOverlap > 0)
		{
			Vec2 vPlayerPos = GetPos();
			vPlayerPos.y -= fOverlap;
			SetPos(vPlayerPos);
		}

		// 땅을 뚫고 올라가는 것을 방지
		Vec2 vVelocity = m_pRigidbody->GetVelocity();
		if (vVelocity.y > 0)
		{
			vVelocity.y = 0.f;
			m_pRigidbody->SetVelocity(vVelocity);
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
	switch (m_eState)
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
