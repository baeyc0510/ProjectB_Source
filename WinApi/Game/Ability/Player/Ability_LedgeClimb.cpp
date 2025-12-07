#include "pch.h"
#include "Ability_LedgeClimb.h"

#include "Game/AnimKey.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CPlayer.h"

Ability_LedgeClimb::Ability_LedgeClimb()
{
}

void Ability_LedgeClimb::OnActivate()
{
	Ability::OnActivate();

	player = dynamic_cast<CPlayer*>(owner);
	if (!player)
	{
		EndAbility();
		return;
	}

	// 컴포넌트 캐시
	rigidbody = owner->GetComponent<CRigidbody>();
	animator = owner->GetComponent<CAnimator>();
	collider = owner->GetComponent<CBoxCollider>();

	// Ledge 정보 캐시
	ledgeX = player->GetLedgeX();
	ledgeTop = player->GetLedgeTop();
	ledgeDirection = player->GetLedgeDirection();

	// 중력 비활성화 및 속도 정지
	rigidbody->UseGravity(false);
	rigidbody->SetVelocity(Vec2(0.f, 0.f));

	// 위치 스냅: 손이 ledge 상단에 닿는 위치로 조정
	// 캐릭터의 상단이 ledgeTop과 일치하도록 설정
	Vec2 colScale = collider->GetScale();
	Vec2 colOffset = collider->GetOffset();
	float colHalfY = colScale.y * 0.5f;

	// X 위치: ledge 가장자리에서 캐릭터 방향 반대로 약간 떨어진 위치
	float snapX = ledgeX - (ledgeDirection * colScale.x * 0.3f);
	// Y 위치: 캐릭터 상단이 ledgeTop에 맞도록
	float snapY = ledgeTop + colHalfY - colOffset.y;

	owner->SetPos(Vec2(snapX, snapY));

	// 애니메이션 방향 설정 및 매달림 애니메이션 재생
	animator->SetDirection(ledgeDirection);
	animator->Play(AnimKey::LedgeHang, true);

	// 이벤트 대기
	WaitEvent(EGameEvent::Input_Up_Hold, BIND_EVENT(this, OnUpHold));
	WaitEvent(EGameEvent::Input_Jump_Pressed, BIND_EVENT(this, OnJumpPressed));

	bClimbingOver = false;
}

void Ability_LedgeClimb::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();

	// 중력 복원
	if (rigidbody)
	{
		rigidbody->UseGravity(true);
	}

	// Ledge 정보 초기화 (재매달림 방지)
	if (player)
	{
		player->ResetLedgeInfo();
	}

	bClimbingOver = false;
}

void Ability_LedgeClimb::OnUpHold()
{
	// 이미 올라가는 중이면 무시
	if (bClimbingOver)
		return;

	bClimbingOver = true;

	// 올라가기 애니메이션 재생 (완료 시 OnClimbOverFinished 호출)
	animator->Play(
		AnimKey::LedgeClimbOver,
		true,
		[this]() { OnClimbOverFinished(); }
	);
}

void Ability_LedgeClimb::OnJumpPressed()
{
	// 올라가는 중이면 무시
	if (bClimbingOver)
		return;

	DropFromLedge();
}

void Ability_LedgeClimb::OnClimbOverFinished()
{
	// 플랫폼 위로 위치 이동
	Vec2 colScale = collider->GetScale();
	Vec2 colOffset = collider->GetOffset();
	float colHalfY = colScale.y * 0.5f;

	// 발이 ledgeTop 위에 오도록 위치 스냅
	float snapX = ledgeX + (ledgeDirection * colScale.x * 0.5f);
	float snapY = ledgeTop - colHalfY - colOffset.y;

	owner->SetPos(Vec2(snapX, snapY));

	EndAbility();
}

void Ability_LedgeClimb::DropFromLedge()
{
	EndAbility();
}
