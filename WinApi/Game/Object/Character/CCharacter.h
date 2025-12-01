#pragma once
#include "Game/Component/CAbilitySystem.h"

class CAbilitySystem;
class CStateSystem;
class CRigidbody;
class CMetaMap;

class CCharacter : public CGameObject
{
public:
    CCharacter();
    ~CCharacter() override;

    virtual wstring GetRandomBloodVfxKey() const;

protected:
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override;
    void OnDisable() override;
    void Release() override;

    void OnCollisionEnter(CCollider* other) override;
    void OnCollisionStay(CCollider* other) override;
    void OnCollisionExit(CCollider* other) override;

    // 공통 로직
    virtual void UpdateGroundState();
    void HandleGroundCollision(CCollider* ground);

    // 메타맵 기반 충돌 처리
    void UpdateMetaCollision();

    // 메타맵 충돌 헬퍼 (방향별 분리)
    struct CollisionContext;
    void ProcessGroundCollision(CollisionContext& ctx);
    void ProcessWallCollision(CollisionContext& ctx, int direction);  // -1: left, 1: right
    void ProcessCeilingCollision(CollisionContext& ctx);

    void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);

    template<typename AbilityType>
    void AddAbility(EAbility AbilityName)
    {
        assert(abilitySystem);
        AbilityType* ability = new AbilityType();
        abilitySystem->AddAbility(AbilityName, ability);
    }

protected:
    // 공통 컴포넌트
    CAnimator* animator;
    CStateSystem* stateSystem;
    CAbilitySystem* abilitySystem;
    CRigidbody* rigidbody;
    CCollider* collider;

    // 공통 상태
    bool bIsGrounded;
};
