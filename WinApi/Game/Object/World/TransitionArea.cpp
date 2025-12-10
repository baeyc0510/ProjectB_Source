#include "pch.h"
#include "TransitionArea.h"

#include "Game/Enum.h"
#include "Game/Component/StateSystem.h"
#include "Game/Manager/GameUIManager.h"
#include "Game/Map/Map.h"
#include "Game/Object/Character/Player.h"
#include "Game/Scene/MapScene.h"
#include "Game/Util/TransitionHelper.h"

TransitionArea::TransitionArea()
{
}

void TransitionArea::InitArea(const SceneTransitionData& transitionData)
{
    // init collider
    Vec2 scale =  Vec2(transitionData.rect.w, transitionData.rect.h);
    collider = AddOrGetComponent<BoxCollider>();
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

void TransitionArea::Init()
{
    collider = AddOrGetComponent<BoxCollider>();
    collider->SetLayer((UINT)ELayer::Transition);
}

void TransitionArea::OnEnable()
{
}

void TransitionArea::Update()
{
}

void TransitionArea::OnDisable()
{
}

void TransitionArea::Release()
{
}

void TransitionArea::Render()
{
}

void TransitionArea::OnCollisionEnter(Collider* other)
{
    if (other->GetLayer() == (UINT)ELayer::Player)
    {
        Scene* scene = SINGLE(SceneManager)->FindScene((int)targetScene);
        if (MapScene* mapScene = dynamic_cast<MapScene*>(scene))
        {
            mapScene->SetSpawnId(spawnId);
        }
        
        const float changeDelay = 0.5f;
        CAMERA->FadeOut(changeDelay);
        WORLD->ChangeScene((int)targetScene, changeDelay);
        GAMEUI->ShowPlayerHUD(false);
        
        // 플레이어 행동을 잠시 멈춤
        if (Player* player = dynamic_cast<Player*>(other->GetOwner()))
        {
            player->GetStateSystem()->AddTag(Tag_BlockMovement);
            player->GetStateSystem()->AddTag(Tag_BlockAbility);
            
            transitionTimerHandle = TIMER->SetTimer([this,player]()
            {
                player->GetStateSystem()->RemoveTag(Tag_BlockMovement);
                player->GetStateSystem()->RemoveTag(Tag_BlockAbility);
            }, changeDelay * 2.0f);
        }
    }
}