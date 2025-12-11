#pragma once
#include "Game/Interface/InteractInterface.h"

class BoxCollider;
class Animator;
class StatComponent;

// 체크포인트 (Prie Dieu)
// - 세이브/리스폰 지점
// - 체력/Flask 회복
class Checkpoint : public GameObject, public IInteractable
{
public:
    Checkpoint();
    ~Checkpoint() override;

    /*~ GameObject Interface ~*/
    void Init() override;
    void Update() override;
    void OnCollisionEnter(Collider* other) override;
    void OnCollisionExit(Collider* other) override;

    /*~ IInteractable Interface ~*/
    bool CanInteract(GameObject* interactor) const override;
    void OnInteract(GameObject* interactor) override;
    const wstring GetInteractionAnimKey(GameObject* interactor) const override;
    const wstring GetInteractionSFXKey(GameObject* interactor) const override;

    /*~ Checkpoint Interface ~*/
    void SetCheckpointID(const wstring& id) { checkpointID = id; }
    wstring GetCheckpointID() const { return checkpointID; }
    bool IsActivated() const { return bIsActivated; }
    void Activate();  // 첫 활성화 (불 점화)

private:
    void ApplyCheckpointEffects(GameObject* player);
    void RestorePlayerResources(StatComponent* playerStat);

private:
    wstring checkpointID;            // 고유 ID (예: "Stage01_CP01")
    bool bIsActivated = false;       // 활성화 여부 (불이 켜졌는지)
    bool bPlayerInRange = false;     // 플레이어가 범위 내에 있는지

    BoxCollider* interactionCollider = nullptr;
    Animator* animator = nullptr;

    // 상수
    static constexpr float INTERACTION_WIDTH = 66.f;
    static constexpr float INTERACTION_HEIGHT = 145.f;
};
