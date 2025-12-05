#pragma once
#include "Game/Component/CAbilitySystem.h"

class CAbilitySystem;
class CStateSystem;
class CRigidbody;
class CCharacterMovement;

class CCharacter : public CGameObject
{
public:
    CCharacter();
    ~CCharacter() override;

    virtual wstring GetRandomBloodVfxKey() const;

    CStateSystem* GetStateSystem() const { return stateSystem; }
    CCharacterMovement* GetMovement() const { return movement; }

    void SetIgnorePlatform(UINT platformID);
    void SetIsGrounded(bool grounded);
    UINT GetCurrentGroundID() const;

    // MovementComponent에서 폴링
    float GetPlatformMinX() const;
    float GetPlatformMaxX() const;
    bool HasPlatformBounds() const;

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

    virtual void UpdateStates();
    virtual void OnStateChanged(EStateTag oldTags, EStateTag newTags);
    virtual bool ShouldIgnorePlatform() const { return false; }

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
    CAnimator* animator = nullptr;
    CStateSystem* stateSystem = nullptr;
    CAbilitySystem* abilitySystem = nullptr;
    CRigidbody* rigidbody = nullptr;
    CBoxCollider* collider = nullptr;
    CCharacterMovement* movement = nullptr;

    // 이전 프레임 상태 (변화 감지용)
    bool bWasOnSteepSlope = false;
};
