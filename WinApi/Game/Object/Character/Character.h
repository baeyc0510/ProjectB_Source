#pragma once
#include "Game/Component/AbilitySystem.h"
#include "Game/Interface/CombatInterface.h"
#include "Game/Enum.h"

enum class EStatType;
class AbilitySystem;
class StateSystem;
class StatComponent;
class Rigidbody;
class CharacterMovement;

class Character : public GameObject
{
public:
    Character();
    ~Character() override;

    /*~ Character Interface ~*/
    StateSystem* GetStateSystem() const { return stateSystem; }
    AbilitySystem* GetAbilitySystem() const { return abilitySystem; }
    StatComponent* GetStatComponent() const { return statComponent; }
    CharacterMovement* GetMovement() const { return movement; }
    virtual wstring GetRandomBloodVfxKey() const;

    void SetIgnorePlatform(UINT platformID);
    void SetIsGrounded(bool grounded);
    bool IsGrounded() const;
    UINT GetCurrentGroundID() const;
    
    float GetPlatformMinX() const;
    float GetPlatformMaxX() const;
    bool HasPlatformBounds() const;

    Vec2 GetPushbackForce() const { return pushbackForce; }

    // 수평 이동 정지 (y속도 유지)
    void StopHorizontalMovement();

    virtual void OnDieStart() {}
    virtual void OnDieComplete() {}

    // VFX Helper
    void SpawnDamageVFX(const CombatContext& context, int spawnDirection);

protected:
    /*~ GameObject Interface ~*/
    void Init() override;
    void OnEnable() override;
    void Update() override;
    void Render() override {}
    void OnDisable() override {}
    void Release() override {}

    void OnCollisionEnter(Collider* other) override;
    void OnCollisionStay(Collider* other) override;
    void OnCollisionExit(Collider* other) override;

    /*~ Character Interface ~*/
    virtual void UpdateStates();
    virtual void OnStateChanged(EStateTag oldTags, EStateTag newTags);
    virtual void OnStatChanged(EStatType type, float& current, float& max);
    virtual void HandleAnimationEvent(EGameEvent event) {}
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
    Animator* animator = nullptr;
    StateSystem* stateSystem = nullptr;
    StatComponent* statComponent = nullptr;
    AbilitySystem* abilitySystem = nullptr;
    Rigidbody* rigidbody = nullptr;
    BoxCollider* collider = nullptr;
    CharacterMovement* movement = nullptr;

    // 이전 프레임 상태 (변화 감지용)
    bool bWasOnSteepSlope = false;
    Vec2 pushbackForce;

private:
    // 이벤트 구독 (RAII 자동 해제)
    SafeDelegateHandle<EStateTag, EStateTag> stateChangedHandle;
    SafeDelegateHandle<EStatType, float, float> statChangedHandle;
};
