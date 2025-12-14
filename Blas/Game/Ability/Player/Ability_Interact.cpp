#include "pch.h"
#include "Ability_Interact.h"
#include "Game/Interface/InteractInterface.h"
#include "Game/Component/Rigidbody.h"
#include "Game/Object/Character/Player.h"


Ability_Interact::Ability_Interact()
{
}

void Ability_Interact::OnActivate()
{
    Ability::OnActivate();
    
    Player* player = dynamic_cast<Player*>(owner);
    if (!player)
    {
        EndAbility();
        return;
    }
    
    // BoxTrace로 주변의 IInteractable 탐색
    Vec2 center = player->GetCenterPos();
    Vec2 halfSize = Vec2(INTERACT_BOX_WIDTH, INTERACT_BOX_HEIGHT) * 0.5f;

    // Default 레이어에서 상호작용 가능한 오브젝트 탐색
    vector<HitResult> hitResults = COLLISION->BoxTrace(center, halfSize, static_cast<UINT>(ELayer::Interaction), true);

    // 가장 가까운 IInteractable 찾기
    IInteractable* closestTarget = nullptr;
    float closestDistance = INTERACT_RANGE;

    for (const auto& result : hitResults)
    {
        GameObject* hitObject = result.collider->GetOwner();
        IInteractable* interactable = dynamic_cast<IInteractable*>(hitObject);

        if (interactable && interactable->CanInteract(owner))
        {
            float distance = (hitObject->GetWorldPos() - center).Length();
            if (distance < closestDistance)
            {
                closestTarget = interactable;
                closestDistance = distance;
            }
        }
    }

    // 상호작용 대상이 없으면 Ability 종료
    if (!closestTarget)
    {
        EndAbility();
        return;
    }

    // 대상 캐싱
    cachedTarget = closestTarget;

    // 속도 정지
    GetRigidbody()->SetVelocity(Vec2(0.f, 0.f));

    auto animKey = closestTarget->GetInteractionAnimKey(owner);
    // 상호작용 애니메이션 재생
    GetAnimator()->Play(animKey,
                        true,
                        BIND(this, OnInteractionComplete),
                        BIND(this, OnInteractionComplete));
    
    // 사운드 재생
    PlaySFX(cachedTarget->GetInteractionSFXKey(owner));

    // DoAction 이벤트 대기
    WaitEvent(EGameEvent::DoAction, BIND_EVENT(this, ExecuteInteraction));
}

void Ability_Interact::OnEnd()
{
    Ability::OnEnd();
    ClearEventHandles();
    cachedTarget = nullptr;
}


void Ability_Interact::ExecuteInteraction()
{
    // 캐싱된 대상과 상호작용
    if (cachedTarget)
    {
        cachedTarget->OnInteract(owner);
    }
}

void Ability_Interact::OnInteractionComplete()
{
    // 애니메이션 종료 후 Ability 종료
    EndAbility();
}
