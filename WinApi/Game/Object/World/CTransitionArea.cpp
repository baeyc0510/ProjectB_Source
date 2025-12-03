#include "pch.h"
#include "CTransitionArea.h"

#include "Game/Enum.h"
#include "Game/Manager/CGameUIManager.h"
#include "Game/Map/CMap.h"
#include "Game/Scene/CMapScene.h"
#include "Game/Util/TransitionHelper.h"

CTransitionArea::CTransitionArea()
{
}

void CTransitionArea::InitArea(const SceneTransitionData& transitionData)
{
    // init collider
    Vec2 scale =  Vec2(transitionData.rect.w, transitionData.rect.h);
    collider = AddOrGetComponent<CBoxCollider>();
    collider->SetScale(scale);
    
    // init pos
    Vec2 pos =  Vec2(transitionData.rect.x, transitionData.rect.y);
    float halfWidth = scale.x * 0.5f;
    float halfHeight = scale.y * 0.5f;
    SetPos(pos + Vec2(halfWidth, halfHeight));
    
    // set metadata
    targetScene = TransitionHelper::ToSceneType(transitionData.targetScene);
    spawnId = transitionData.spawnId;
}

void CTransitionArea::Init()
{
    collider = AddOrGetComponent<CBoxCollider>();
    collider->SetLayer(ELayer::Transition);
}

void CTransitionArea::OnEnable()
{
}

void CTransitionArea::Update()
{
}

void CTransitionArea::OnDisable()
{
}

void CTransitionArea::Release()
{
}

void CTransitionArea::Render()
{
}

void CTransitionArea::OnCollisionEnter(CCollider* other)
{
    if (other->GetLayer() == ELayer::Player)
    {
        CScene* scene = SINGLE(CSceneManager)->FindScene(targetScene);
        if (CMapScene* mapScene = dynamic_cast<CMapScene*>(scene))
        {
            mapScene->SetSpawnId(spawnId);
        }
        
        CAMERA->FadeOut(0.5f);
        WORLD->ChangeScene(targetScene, 0.5f);
        GAMEUI->ShowHUD(false);
    }
}