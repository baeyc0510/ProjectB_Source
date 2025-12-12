#pragma once

class TransitionArea;
class Player;

class MapScene : public Scene
{
public:
    MapScene();
    ~MapScene() override;
    
    /*~ MapScene Interface ~*/
    Player* SpawnPlayer();
    void SetSpawnId(int inSpawnId) { spawnId = inSpawnId; }
    
protected:
    /*~ Scene Interface ~*/
    void Init()		override;
    void Enter()	override;
    void Update()	override;
    void Render()	override;
    void Exit()		override;
    void Release()	override;
    
    // 맵 레이어 렌더링
    void RenderBackground() override;
    void RenderForeground() override;
    
    /*~ MapScene Interface ~*/
    void SpawnTransitionArea();
    virtual void OnLoadMap();

protected:
    wstring mapFilePath;
    int spawnId;
};
