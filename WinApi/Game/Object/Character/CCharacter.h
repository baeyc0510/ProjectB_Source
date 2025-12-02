#pragma once
#include "Game/Component/CAbilitySystem.h"

class CAbilitySystem;
class CStateSystem;
class CRigidbody;
class CMetaMap;

// 충돌 처리에 필요한 데이터
struct CollisionContext
{
    CMetaMap* metaMap;
    Vec2 pos;               // 캐릭터 위치 (수정 가능)
    Vec2 pixelCenter;       // 콜라이더 중심 (픽셀 좌표)
    Vec2 colliderOffset;
    float halfWidth;
    float halfHeight;
    bool wasGrounded;
};

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
    virtual void UpdateStates();
    // 메타맵 기반 충돌 처리
    void UpdateMetaCollision();

    // 메타맵 충돌 헬퍼
    void ProcessGroundCollision(CollisionContext& ctx);
    void ProcessWallCollision(CollisionContext& ctx, int direction);  // -1: left, 1: right
    void ProcessCeilingCollision(CollisionContext& ctx);
    virtual void ProcessMetaCollision(CollisionContext& ctx) {} // 자식 클래스 커스텀 로직용

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
