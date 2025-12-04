#pragma once
#include "Game/Component/CAbilitySystem.h"

class CAbilitySystem;
class CStateSystem;
class CRigidbody;
class CMetaMap;

class CCharacter : public CGameObject
{
public:
    // 슬로프 관련 상수
    static constexpr float MAX_SLOPE_ANGLE = 50.0f;  // 오를 수 있는 최대 각도 (도)
    static constexpr float MAX_SLOPE_ANGLE_RAD = MAX_SLOPE_ANGLE * 3.14159265f / 180.0f;  // 라디안

public:
    CCharacter();
    ~CCharacter() override;

    virtual wstring GetRandomBloodVfxKey() const;

    CStateSystem* GetStateSystem() const { return stateSystem; }

    void SetIgnorePlatform(UINT platformID) { ignoredPlatformID = platformID; }
    void SetIsGrounded(bool inIsGrounded);
    UINT GetCurrentGroundID() const;  // 현재 서있는 지면/플랫폼 ID (없으면 0)

    // 현재 서있는 플랫폼의 X축 경계
    float GetPlatformMinX() const { return platformMinX; }
    float GetPlatformMaxX() const { return platformMaxX; }
    bool HasPlatformBounds() const { return activeGroundID != 0; }
    
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
    
    void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);

    template<typename AbilityType>
    void AddAbility(EAbility AbilityName)
    {
        assert(abilitySystem);
        AbilityType* ability = new AbilityType();
        abilitySystem->AddAbility(AbilityName, ability);
    }
    
private:
    void SetIsGrounded_Internal(bool inIsGrounded);

protected:
    // 공통 컴포넌트
    CAnimator* animator;
    CStateSystem* stateSystem;
    CAbilitySystem* abilitySystem;
    CRigidbody* rigidbody;
    CBoxCollider* collider;

    // 공통 상태
    bool bIsGrounded;       // 지면 착지 상태
    bool bIsOnSteepSlope;   // 가파른 경사면에서 미끄러지는 중
    bool bWasOnSteepSlope;  // 이전 프레임 상태 (태그 변화 감지용)
    UINT ignoredPlatformID; // 통과 중인 플랫폼 ID (0이면 없음)
    UINT activeGroundID;    // 실제로 서있는 지면/플랫폼 ID (스냅 대상)
    float activeGroundTop;  // activeGround 플랫폼의 상단 Y좌표 (비교용, Y가 클수록 아래)
    float platformMinX;     // 현재 플랫폼 X 최소값
    float platformMaxX;     // 현재 플랫폼 X 최대값
};
