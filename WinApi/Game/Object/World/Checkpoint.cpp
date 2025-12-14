#include "pch.h"
#include "Checkpoint.h"

#include "Game/Component/StatComponent.h"
#include "Game/Data/AnimKey.h"
#include "Game/Data/SFXKeys.h"
#include "Game/Manager/SaveManager.h"
#include "Game/Object/Character/Character.h"
#include "Game/Scene/MapScene.h"
#include "Game/Util/AnimationHelper.h"

Checkpoint::Checkpoint()
{
    name = TEXT("Checkpoint");
}

Checkpoint::~Checkpoint()
{
}

void Checkpoint::Init()
{
    // Interaction Collider 생성
    interactionCollider = new BoxCollider;
    interactionCollider->SetScale(Vec2(INTERACTION_WIDTH, INTERACTION_HEIGHT));
    interactionCollider->SetLayer(static_cast<UINT>(ELayer::Interaction));
    interactionCollider->SetOffset(Vec2(0, -INTERACTION_HEIGHT / 2.0f));
    AddChild(interactionCollider);

    // Animator 생성
    animator = new Animator;
    AddChild(animator);
    
    AnimationHelper::AddAnimation(animator,AnimKey::Idle,TEXT("Animations/Interactable/priedieu_off.json"), true);
    AnimationHelper::AddAnimation(animator,AnimKey::ActivateInteractable,TEXT("Animations/Interactable/priedieu_on.json"), true);

    // 초기 상태: 비활성화
    bIsActivated = false;
    bPlayerInRange = false;
}

void Checkpoint::Update()
{
    GameObject::Update();
    
    if (bIsActivated)
    {
        animator->Play(AnimKey::ActivateInteractable, false);
    }
    else
    {
        animator->Play(AnimKey::Idle, true);
    }
}

void Checkpoint::OnCollisionEnter(Collider* other)
{
    if (other->GetLayer() == static_cast<UINT>(ELayer::Player))
    {
        bPlayerInRange = true;
    }
}

void Checkpoint::OnCollisionExit(Collider* other)
{
    if (other->GetLayer() == static_cast<UINT>(ELayer::Player))
    {
        bPlayerInRange = false;
    }
}

bool Checkpoint::CanInteract(GameObject* interactor) const
{
    return bPlayerInRange;
}

void Checkpoint::OnInteract(GameObject* interactor)
{
    // 플레이어의 StatComponent 찾기
    Character* player = dynamic_cast<Character*>(interactor);
    if (!player)
        return;

    StatComponent* playerStat = player->GetStatComponent();
    if (!playerStat)
        return;

    // 체크포인트 효과 적용
    Activate();
    ApplyCheckpointEffects(interactor);
    RestorePlayerResources(playerStat);
}

const wstring Checkpoint::GetInteractionAnimKey(GameObject* interactor) const
{
    return AnimKey::Pray;
}

const wstring Checkpoint::GetInteractionSFXKey(GameObject* interactor) const
{
    return SFXKey::PlayerActivatePrayer;
}

void Checkpoint::Activate()
{
    if (bIsActivated)
        return;

    bIsActivated = true;
}

void Checkpoint::ApplyCheckpointEffects(GameObject* player)
{
    // 모든 게임 씬 리셋 예약 (적 리스폰) - 프레임 끝에서 안전하게 처리
    WORLD->ResetAllScenes();

    // 체크포인트 저장
    SAVE->SaveCheckpoint(
        checkpointID,
        GetPos(),
        (int)SINGLE(SceneManager)->GetCurSceneKey()
    );
}

void Checkpoint::RestorePlayerResources(StatComponent* playerStat)
{
    // 체력 완전 회복
    float maxHP = playerStat->GetMax(EStatType::HP);
    playerStat->SetCurrent(EStatType::HP, maxHP);

    float maxMP = playerStat->GetMax(EStatType::MP);
    playerStat->SetCurrent(EStatType::MP, maxMP);
    
    // Flask 전량 회복
    float maxFlask = playerStat->GetMax(EStatType::Flask);
    playerStat->SetCurrent(EStatType::Flask, maxFlask);

    // TODO: VFX/SFX 재생
    // PlaySFX(SFXKey::CheckpointPrayer);
}
