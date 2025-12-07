#include "pch.h"
#include "Ability_BossStomp.h"

#include "Game/AnimKey.h"
#include "Game/VFXKeys.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"

void Ability_BossStomp::OnActivate()
{
	Ability::OnActivate();

	GetAnimator()->Play(AnimKey::BossStomp, true, BIND(this, EndAbility), BIND(this, EndAbility));

	WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
}

void Ability_BossStomp::OnEnd()
{
	Ability::OnEnd();
	ClearEventHandles();
}

void Ability_BossStomp::OnHitCheck()
{
	Vec2 offset = GetTraceOffset();
	Vec2 center = owner->GetWorldPos() + offset;
	Vec2 size = GetTraceSize();

	auto results = COLLISION->BoxTrace(center, size, ELayer::Player, true);
	for (auto& result : results)
	{
		CGameObject* player = result.collider->GetOwner();

		if (IsPlayerGuarding(player))
		{
			// 가드 중이면 밀어내기만
			PushbackPlayer(player);
		}
		else
		{
			// 가드 안하면 데미지 + 날리기 + 가시 생성
			ICombatInterface* combat = dynamic_cast<ICombatInterface*>(player);
			if (combat)
			{
				CombatContext context;
				context.damageType = EDamageType::Slash;
				context.hitResult = result;
				context.value = DAMAGE;
				context.vfxKey = VFXKey::AttackHit1;
				combat->OnDamage(owner, context);
			}

			LaunchPlayer(player);
		}
	}
}

bool Ability_BossStomp::IsPlayerGuarding(CGameObject* player) const
{
	CStateSystem* playerState = player->GetComponent<CStateSystem>();
	if (!playerState)
		return false;

	// 패리 중이면 가드 성공으로 처리
	return playerState->HasTag(Tag_Parrying);
}

void Ability_BossStomp::LaunchPlayer(CGameObject* player)
{
	CRigidbody* playerRb = player->GetComponent<CRigidbody>();
	if (!playerRb)
		return;

	// 보스 방향으로 플레이어 날리기
	float dir = static_cast<float>(owner->GetForward());
	playerRb->SetVelocity(Vec2(LAUNCH_FORCE_X * dir, LAUNCH_FORCE_Y));

	// TODO: 착지 예상 위치에 가시 생성 (추후 Hazard 시스템 구현 후)
	// Vec2 landingPos = PredictLandingPosition(player);
	// ScheduleSpikeSpawn(landingPos);
}

void Ability_BossStomp::PushbackPlayer(CGameObject* player)
{
	CRigidbody* playerRb = player->GetComponent<CRigidbody>();
	if (!playerRb)
		return;

	// 보스 방향으로 플레이어 밀어내기
	float dir = static_cast<float>(owner->GetForward());
	playerRb->SetVelocity(Vec2(PUSHBACK_FORCE * dir, 0.f));
}

void Ability_BossStomp::ScheduleSpikeSpawn(Vec2 landingPos)
{
	// TODO: Hazard 시스템 구현 후 가시 생성 로직 추가
	// 플레이어 착지 예상 위치에 가시를 생성
}

Vec2 Ability_BossStomp::GetTraceOffset() const
{
	Vec2 offset(60.f, -40.f);
	offset.x *= owner->GetForward();
	return offset;
}

Vec2 Ability_BossStomp::GetTraceSize() const
{
	return Vec2(80.f, 60.f);
}
