#pragma once
#include "Game/Enum.h"

struct SceneTransitionData;

class TransitionArea : public CGameObject
{
public:
    TransitionArea();
    
    /*~ CTransitionArea Interface ~*/
    void InitArea(const SceneTransitionData& transitionData);
    
protected:
    /*~ CGameObject Interface ~*/
    void Init()	override;
    void OnEnable()	override;
    void Update() override;
    void OnDisable() override;
    void Release() override;
    void Render() override;
    
    void OnCollisionEnter(CCollider* other) override;

private:
    CBoxCollider* collider;
    ESceneType targetScene;
    int spawnId;
    
    SafeTimerHandle transitionTimerHandle;
};
