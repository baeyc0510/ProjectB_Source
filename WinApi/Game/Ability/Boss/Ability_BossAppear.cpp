#include "pch.h"
#include "Ability_BossAppear.h"

#include "Game/AnimKey.h"
#include "Game/SFXKeys.h"
#include "Game/CGame.h"
#include "Game/Component/CBossAIController.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Object/Character/CBoss.h"
#include "Game/Object/Character/CPlayer.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"

void Ability_BossAppear::OnActivate()
{
    Ability::OnActivate();
    
    boss = dynamic_cast<CBoss*>(GetOwner());
    if (!boss)
    {
        EndAbility();
        return;
    }
    
    bossAI = boss->GetBossAI();
    if (!bossAI)
    {
        EndAbility();
        return;
    }
    
    player = dynamic_cast<CPlayer*>(bossAI->GetTarget());
    if (!player)
    {
        EndAbility();
        return;
    }
    
    GetAnimator()->Play(AnimKey::BossAppear,true, BIND(this,EndAbility));
    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitGround));
    WaitEvent(EGameEvent::DoAction, BIND_EVENT(this,PlayWakeUpSound));
    
    // 카메라 연출
    CAMERA->SetSmoothSpeed(1.f);
    CAMERA->SetTargetObj(nullptr);
    CAMERA->SetTargetPos(owner->GetPos());
    
    // 플레이어 정지
    player->GetComponent<CRigidbody>()->SetVelocity(Vec2(0.f,0.f));
    player->GetStateSystem()->AddTag(Tag_BlockMovement);
    player->GetAbilitySystem()->CancelAbilitiesWithTag(Tag_Moving);
}

void Ability_BossAppear::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
    
    if (!boss || !bossAI)
    {
        return;
    }
    
    boss->OnAppearanceComplete();
    
    // 플레이어 비추게 돌아감
    CGameObject* playerObj = bossAI->GetTarget();
    CAMERA->SetTargetPos(bossAI->GetTarget()->GetPos() + CGame::DEFAULT_CAMERA_OFFSET);
    
    // 2초후
    TIMER->SetTimer([this, playerObj]()
    {
        // 플레이어 정지 해제
        player->GetStateSystem()->RemoveTag(Tag_BlockMovement);
        
        // 카메라 세팅 기본값으로 리셋
        CAMERA->SetTargetObj(playerObj);
        CAMERA->SetSmoothSpeed(CGame::DEFAULT_CAMERA_SMOOTH);
    }, 2.0f);
}

void Ability_BossAppear::PlayWakeUpSound()
{
    // SFX 재생
    PlaySFX(SFXKey::PiedadWakeUp);
}

void Ability_BossAppear::OnHitGround()
{
    ShakeCamera(ShakePreset::Heavy);
}
