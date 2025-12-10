#pragma once
#include "Game/Enum.h"

struct SceneTransitionData;

class TransitionArea : public GameObject
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
    
    void OnCollisionEnter(Collider* other) override;

private:
    BoxCollider* collider;
    ESceneType targetScene;
    int spawnId;
    
    SafeTimerHandle transitionTimerHandle;
};
