#include "pch.h"
#include "CPlayer.h"

CPlayer::CPlayer()
{
	name		= TEXT("플레이어");
	scale		= Vec2(100, 100);
	animator	= nullptr;
	speed		= 200.f;

	moveDir		= Vec2(0, 0);
	lookDir		= Vec2(0, -1);
	isMove		= false;
}

CPlayer::~CPlayer()
{
}

void CPlayer::Init()
{
	// 애니메이션
	CAnimation* idleAnimation = LOADANIMATION(TEXT("PlayerIdle"), TEXT("Animations/Player_Idle.json"));
	idleAnimation->SetRepeat(true);
	
	CAnimation* runAnimation = LOADANIMATION(TEXT("PlayerRun"), TEXT("Animations/Player_Run.json"));
	runAnimation->SetRepeat(true);

	animator = new CAnimator();

	animator->AddAnimation(TEXT("Idle"), idleAnimation);
	animator->AddAnimation(TEXT("Run"), runAnimation);

	AddChild(animator);

	// Collider
	CCollider* collider = new CCollider();
	collider->SetScale(Vec2(90, 90));
	collider->SetLayer(Layer::Player);

	AddChild(collider);
}

void CPlayer::OnEnable()
{
}

void CPlayer::Update()
{
	isMove = false;

	// 이동
	if (INPUT->ButtonStay(VK_LEFT))
	{
		pos.x -= speed * DT;
		isMove = true;
		moveDir.x = -1;
	}
	else if (INPUT->ButtonStay(VK_RIGHT))
	{
		pos.x += speed * DT;
		isMove = true;
		moveDir.x = +1;
	}
	else
	{
		moveDir.x = 0;
	}

	if (INPUT->ButtonStay(VK_UP))
	{
		pos.y -= speed * DT;
		isMove = true;
		moveDir.y = +1;
	}
	else if (INPUT->ButtonStay(VK_DOWN))
	{
		pos.y += speed * DT;
		isMove = true;
		moveDir.y = -1;
	}
	else
	{
		moveDir.y = 0;
	}

	AnimatorUpdate();
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

void CPlayer::AnimatorUpdate()
{
	if (moveDir.Length() > 0)
		lookDir = moveDir;

	wstring str = TEXT("");

	if (isMove)	str += TEXT("Move");
	else			str += TEXT("Idle");

	if (lookDir.x > 0) str += TEXT("Right");
	else if (lookDir.x < 0) str += TEXT("Left");

	if (lookDir.y > 0) str += TEXT("Up");
	else if (lookDir.y < 0) str += TEXT("Down");

	animator->Play(str, false);
}