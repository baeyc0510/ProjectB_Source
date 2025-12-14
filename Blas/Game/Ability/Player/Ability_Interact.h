#pragma once
#include "Game/Ability/Ability.h"

class Ability_Interact : public Ability
{
public:
    Ability_Interact();

    /*~ Ability Interface ~*/
    EStateTag GetRequiredTags() const override { return Tag_Grounded; }
    EStateTag GetBlockedTags() const override { return Tag_BlockAbility | Tag_BlockMovement; }
    EStateTag GetTagsToAdd() const override { return Tag_BlockMovement | Tag_BlockAbility  | Tag_AbilityAnimation; }

    void OnActivate() override;
    void OnEnd() override;

private:
    void ExecuteInteraction();  // DoAction 이벤트 발생 시 실제 상호작용 실행
    void OnInteractionComplete();  // 애니메이션 종료 시 호출

private:
    static constexpr float INTERACT_RANGE = 80.f;  // 상호작용 탐색 범위
    static constexpr float INTERACT_BOX_WIDTH = 100.f;
    static constexpr float INTERACT_BOX_HEIGHT = 100.f;

    class IInteractable* cachedTarget = nullptr;  // OnActivate에서 찾은 대상 캐싱
};
