#pragma once
#include "Game/Component/CAbilitySystem.h"

class CAbilitySystem;
class CStateSystem;

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
    
    void AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat);
    
    template<typename AbilityType>
    void AddAbility(EAbility AbilityName)
    {
        assert(abilitySystem);
        AbilityType* ability = new AbilityType();
        abilitySystem->AddAbility(AbilityName, ability);
    }
    
protected:
    CAnimator* animator;
    CStateSystem* stateSystem;
    CAbilitySystem* abilitySystem;
};
